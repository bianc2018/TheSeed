/*
    ��Ϣ������
    �������� Ȼ�� ���message
*/
#ifndef MESSAGE_PARSER_H_
#define MESSAGE_PARSER_H_
#include <queue>
namespace msg
{
    //class BuffCache
    enum ParserStatus
    {
        //���ҿ�ʼ
        PARSE_FIND_START = 0,
        //�����
        PARSE_FIND_END = 1,
        PARSE_ERROR = -1,
    };
    template<typename MessageType>
    class MessageParser
    {
    public:
        MessageParser()
        {
            reset();
        }

        //����
        std::shared_ptr<MessageType> get_message()
        {
            if (message_queue_.empty())
                return nullptr;

            std::shared_ptr<MessageType> message = message_queue_.front();
            message_queue_.pop();
            return message;
        }

        //����
        ParserStatus parse(const char* buff, size_t len)
        {
            if (PARSE_FIND_START == parser_status_)
                reset();

            //LOG_INFO <<"buff="<<std::string(buff,len);
            for (auto i = 0; i < len; ++i)
            {
                auto ch_p = buff[i];
                if (MASK == ch_p)
                {
                    if (PARSE_FIND_START == parser_status_)
                    {
                        reset();
                        parser_status_ = PARSE_FIND_END;
                    }
                    else  if (PARSE_FIND_END == parser_status_)
                    {
                        if ("" == parser_cache1_)
                            reset();
                        else
                        {
                            try
                            {
                                std::shared_ptr<MessageType> now = std::make_shared<MessageType>();
                                if (now->from_string(parser_cache1_))
                                {
                                    add_message(now);
                                    reset();
                                }
                                else
                                {
                                    parser_status_ = PARSE_ERROR;
                                    return  parser_status_;
                                }
                            }
                            catch (std::exception & e)
                            {
                                parser_status_ = PARSE_ERROR;

                                return  parser_status_;
                            }
                        }
                    }
                }
                else
                {
                    parser_cache1_ += ch_p;
                }

            }

            return parser_status_;
        }


        //����״̬
        void reset()
        {
            parser_cache1_ = "";
            parser_status_ = PARSE_FIND_START;
        }

        //��ȡδ����Ļ���
        std::string get_cache()
        {
            auto d = std::move(parser_cache1_);
            reset();
            return d;

        }

    private:

        void add_message(std::shared_ptr<MessageType> message)
        {
            message_queue_.push(message);
        }

       
    private:
        //����״̬
        ParserStatus parser_status_;

        //��������
        std::string parser_cache1_;

        //�Ѿ����ɵĶ��л���
        std::queue<std::shared_ptr<MessageType>> message_queue_;
    };
}

#endif