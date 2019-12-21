/*
    �������ݽṹ
*/
#ifndef MESSAGE_H_
#define MESSAGE_H_
#include <string>
#include <unordered_map>
#include <sstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>

#include "logging.hpp"

namespace msg
{
    //1�ֽ� 8 0xff std::uint8_t
    typedef uint8_t BYTE;
    //2�ֽ� 16 0xffff std::uint16_t
    typedef uint16_t WORD;
    //4�ֽ� 32 0xffff ffff std::uint32_t
    typedef uint32_t DWORD;
    //8�ֽ� 64 0xffff ffff ffff ffff std::uint64_t 
    typedef std::uint64_t BIT64;
    //Message
    /*
        0x7e <-> 0x7d 0x01
        0x7d <-> 0x7d 0x02

        0x7e
        head
        body
        check_code
        0x7e
    */
    //�ָ��
    const BYTE MASK(0x7e);
    const BYTE ESCAPE(0x7d);
    
    //��ϢЭ��汾
    const DWORD MESSAGE_VERSION(0x00010000);

    struct MessageHead
    {
        //url
        std::string path;
        
        //ԭ��ַ
        std::string src;

        //Ŀ�ĵ�ַ
        std::string dst;

        //��չ�ֶ�
        std::map<std::string, std::string> extend_head;

        MessageHead()
        {
          
        }
    };

    //��������
    template<typename ...Types>
    struct MessageData
    {
        //�̶�ͷ
        MessageHead head;

        //������
        boost::variant<Types...> body;
    };

    //����
    template<typename ...Types>
    class Message
    {
    public:
        Message()=default;

        bool from_string(const std::string& str)
        {
            auto decode_str = decode(str);
            if ("" == decode_str)
            {
                LOG_ERR << "decode error str =" << str;
                return false;
            }
            if (!check_code((const BYTE*)str.c_str(), str.size() - 1, str[str.size() - 1]))
            {
                LOG_ERR << "check_code error str =" << str << ",check_code=" << str[str.size() - 1];
                return false;
            }
            try
            {
                // std::stringstream ss(str.substr(0, str.size() - 1));
                std::stringstream ss(str);
                boost::archive::binary_iarchive iarchive(ss);
                iarchive& data_;
                return true;
            }
            catch (const std::exception & e)
            {
                LOG_ERR << " from_string exception what=" << e.what();
                return false;
            }
        }
        
        std::string to_string()
        {
            try
            {
                // std::stringstream ss(str.substr(0, str.size() - 1));
                std::stringstream ss;
                boost::archive::binary_oarchive oarchive(ss);
                oarchive& data_;
                //����
                BYTE check_code = 0;
                if (!get_check_code((BYTE*)ss.str().c_str(), ss.str().size(), check_code))
                {
                    LOG_ERR << "get_check_code error ";
                    return "";
                }
                ss << (char)check_code;
                //����
                auto str = encode(ss.str());
                if ("" == str)
                {
                    LOG_ERR << "encode error ";
                    return "";
                }
                return (char)msg::MASK + str + (char)msg::MASK;
            }
            catch (const std::exception & e)
            {
                LOG_ERR << " to_string exception what=" << e.what();
                return "";
            }
        }

        MessageHead& get_head()
        {
            return data_.head;
        }

        //��ȡ������
        template<typename body_type>
        bool getbody(body_type& body)
        {
            try
            {
                body = boost::get<body_type>(data_.body);
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

        template<typename body_type>
        bool setbody(body_type& body)
        {
            try
            {
                data_.body = body;
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }
    private:
        /*
        0x7e <-> 0x7d 0x01
        0x7d <-> 0x7d 0x02
        */
        //����� ʧ�ܷ��ؿ�
        static std::string encode(const std::string& in)
        {
            std::string en_str;
            for (auto& c : in)
            {
                if (MASK == c)
                    en_str += {0x7d, 0x01};
                else if (ESCAPE == c)
                    en_str += {0x7d, 0x02};
                else
                    en_str += c;
            }
            return en_str;
        }

        static std::string decode(const std::string& in)
        {
            std::string de_str;
            //�Ƿ�����ת����
            bool escape_mask = false;
            for (auto& c : in)
            {
                if (escape_mask)
                {
                    if (0x01 == c)
                    {
                        de_str += (char)MASK;
                    }
                    else if (0x02 == c)
                    {
                        de_str += (char)ESCAPE;
                    }
                    else
                    {
                        LOG_ERR << "decode error c=" << (int)c;
                        return "";
                    }
                    escape_mask = false;
                }
                else
                {
                    if (ESCAPE == c)
                        escape_mask = true;
                    else
                        de_str += c;
                }

            }
            return de_str;
        }

        //���У��
        static bool check_code(const BYTE* data, size_t data_len, const BYTE& check_code)
        {
            BYTE now_check_code = 0;
            get_check_code(data, data_len, now_check_code);
            return now_check_code == check_code;
        }

        //����У����
        static bool get_check_code(const BYTE* data, size_t data_len, BYTE& check_code)
        {
            for (size_t i = 0; i < data_len; ++i)
            {
                //xor
                check_code = check_code ^ data[i];
            }
            return true;
        }

    private:
        MessageData<Types...> data_;
    };
}

//���л�
namespace boost {
    namespace serialization
    {
        template<typename Archive>
        void serialize(Archive& ar, msg::MessageHead& head, const unsigned int version)
        {
            ar& head.path;
            ar& head.dst;
            ar& head.src;
            ar& head.extend_head;
        }

        template<typename Archive, typename ...Types>
        void serialize(Archive& ar, msg::MessageData< Types...>& msg, const unsigned int version)
        {
            ar& msg.head;
            ar& msg.body;
        }
    }
}

#endif
