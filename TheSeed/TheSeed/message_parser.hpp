/*
    消息解析器
    输入数据 然后 输出message
*/
#ifndef MESSAGE_PARSER_H_
#define MESSAGE_PARSER_H_
#include <queue>
namespace msg
{
    //class BuffCache
    enum ParserStatus
    {
        //查找开始
        PARSE_FIND_START = 0,
        //与结束
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

        //数据
        std::shared_ptr<MessageType> get_message()
        {
            if (message_queue_.empty())
                return nullptr;

            std::shared_ptr<MessageType> message = message_queue_.front();
            message_queue_.pop();
            return message;
        }

        //解析
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


        //重置状态
        void reset()
        {
            parser_cache1_ = "";
            parser_status_ = PARSE_FIND_START;
        }

        //获取未处理的缓存
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
        //解析状态
        ParserStatus parser_status_;

        //解析缓存
        std::string parser_cache1_;

        //已经生成的队列缓存
        std::queue<std::shared_ptr<MessageType>> message_queue_;
    };
}

#endif