/*
    �������ݽṹ
*/
#ifndef MESSAGE_H_
#define MESSAGE_H_
#include <string>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include "ns_cache.hpp"

namespace msg
{
    /*
        ����Э�飺
        ͷ�ֶ�
            Method 
                ����
                ���� ѡ�� Response �ظ����� request ������
            Cmd
                ָ��
                ����ʱ���Զ���ı�����Ϣ
                body ����Ϊ����

            Token
                �����ʾ 
                ÿ������ �ж�����token ��û��token�ֶα�ʾ����Ҫ����˻ظ���ÿ���ظ�����Я��token�ֶΣ���ʶ��ظ�������

            BodyLen
                �����峤��
                ��ʶ ÿһ�����ı������ݵĳ��ȣ�BodyLen=0 ��ʶΪ��

            Close��
                �رձ�ʶ 
                true ��ʶ �Ự�ѷ��� false �Ự���� Ĭ�� Ϊtrue
    */

    //�����ֶ� 

    //����
    const std::string METHOD = "Method";
    //֧�ֵķ��� �ظ� token ������
    const std::string METHOD_Response = "Response";
    //�� Ŀ��ڵ� ��ȡ����
    const std::string METHOD_Request = "Request";

    //ָ��
    const std::string CMD = "Cmd";
    
    //Դ�ڵ�ip �˿� Ĭ�� 5555
    const std::string SRC_IP = "SrcIp";
    const std::string SRC_PORT = "SrcPort";
    const std::string DST_IP = "DstIp";
    const std::string DST_PORT = "DstPort";
    //��ʶ
    const std::string TOKEN = "Token";
    //body ����
    const std::string BODY_LEN = "BodyLen";

    //���� true or false
    const std::string Close = "Close";

    const std::string CRLF("\r\n");
    const std::string CRLFCRLF("\r\n\r\n");
    const std::string SPACE(" ");
    const std::string HEAD_SPLIT(": ");

    class Message
    {
    public:
        //��ȡbody����
        size_t get_body_len();

        //��ȡ����
        util::BufferCache& get_body();

        //��ȡֵ
        std::string get_head_value(const std::string& key, const std::string& notfond = "");

        //��ȡֵ
        template<typename T>
        T get_head_value(const std::string& key, const T& notfond)
        {
            std::string v = get_head_value(key);
            if (v == "")
                return notfond;
            try
            {
                auto p = boost::lexical_cast<T>(v);
                return  p;
            }
            catch (std::exception & e)
            {
                LOG_ERR << "lexical_cast error,what=" << e.what();
                return notfond;
            }
        }
        //���ֵ���� �򲻸���
        bool set_head_value_not_over(const std::string& key, const std::string& value);
        
        //����
        bool set_head_value(const std::string& key, const std::string& value);

        std::string get_head();

    public:
        //��������

        //����
        bool set_method(const std::string& method);
        std::string get_method(const std::string& not_find="");

        //ָ��
        bool set_cmd(const std::string& cmd);
        std::string get_cmd(const std::string& not_find = "");

        //ָ��
        bool set_token(const std::string& token);
        std::string get_token(const std::string& not_find = "");

        //����Դ�ڵ�
        bool set_src_node(const std::string& ip,int port);
        std::pair<std::string, int> get_src_node(\
            const std::pair<std::string, int> &not_find = { "",-1 });

        //����Ŀ�Ľڵ�
        bool set_dst_node(const std::string& ip, int port);
        std::pair<std::string, int> get_dst_node(\
            const std::pair<std::string, int>& not_find = { "",-1 });
        
        //�رձ��
        bool set_close(bool flag);
        bool get_close(const bool& not_find = true);

        //�����ļ�Ϊbody����
        bool set_body_from_file(const std::string& file_path);

        //��body�������Ϊ�ļ�
        bool save_body_as_file(const std::string& file_path);

        //дbody
        bool write_body(const char* data, size_t len);

        //��body ���ض�ȡ���ַ���Ŀ
        std::int64_t read_body(char* buff, size_t len);
    private:
        //key - value
        //�ṹ���ı���ͷ key:value
        std::unordered_map<std::string, std::string> header_;

        //��������
        util::BufferCache body_;
    };
}
#endif
