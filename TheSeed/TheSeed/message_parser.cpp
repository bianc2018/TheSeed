#include "message_parser.h"
using namespace msg;
msg::MessageParser::MessageParser():parser_status_(PARSE_INIT), content_len_cache_(0), now_message_(nullptr)
{
}

std::shared_ptr<Message> msg::MessageParser::get_message()
{
    if (message_queue_.empty())
        return nullptr;

    std::shared_ptr<Message> message = message_queue_.front();
    message_queue_.pop();
    return message;
}

ParserStatus msg::MessageParser::parse(const char* buff, size_t len)
{
    
    //LOG_INFO <<"buff="<<std::string(buff,len);
    for (auto i = 0; i < len; ++i)
    {
        auto ch_p = buff[i];
        
        if (PARSE_INIT == parser_status_|| PARSE_ERROR == parser_status_)
        {
            parser_cache1_ = "";
            parser_cache2_ = "";
            content_len_cache_ = 0;
            now_message_ = std::make_shared<Message>();
            parser_status_ = PARSE_HEAD_KEY;
        }

        if (PARSE_HEAD_KEY == parser_status_)
        {

            if ('\r' == ch_p)
            {
                continue;
            }
            else if ('\n' == ch_p)
            {
                if (parser_cache1_ != "")
                {
                    parser_status_ = PARSE_ERROR;
                    
                    return parser_status_;
                }

                auto len = now_message_->get_head_value(BODY_LEN, "");
                if ("0" == len||"" == len)//body =0
                {
                    parser_status_ = PARSE_INIT;
                    set_message(now_message_);
                }
                parser_status_ = PARSE_BODY;
            }
            else if (':' == ch_p)
            {
                parser_status_ = PARSE_HEAD_VALUE;
            }
            else
            {
                parser_cache1_ += ch_p;
            }
        }
        else if (PARSE_HEAD_VALUE == parser_status_)
        {
            if ('\r' == ch_p)
            {
                continue;
            }
            if (' ' == ch_p)
            {
                //键值为空
                if ("" == parser_cache2_)
                    continue;
                else
                    parser_cache2_ += ch_p;
            }
            else if ('\n' == ch_p)
            {
                //键值为空
                if ("" == parser_cache1_)
                {
                    parser_status_ = PARSE_ERROR;
                    return parser_status_;
                }
                //缓存长度
                if ((BODY_LEN == parser_cache1_) && ("" != parser_cache2_))
                {
                    try
                    {
                        content_len_cache_ = std::stoll(parser_cache2_);

                    }
                    catch (std::exception & e)
                    {
                        parser_status_ = PARSE_ERROR;
                        return PARSE_ERROR;
                    }
                }
               // LOG_DBG << parser_cache1_ << ":" << parser_cache2_;
                now_message_->set_head_value(parser_cache1_, parser_cache2_);
                parser_cache1_ = "";
                parser_cache2_ = "";
                parser_status_ = PARSE_HEAD_KEY;
            }
            else
            {
                parser_cache2_ += ch_p;
            }
        }
        else if (PARSE_BODY == parser_status_)
        {
            if (!now_message_->get_body().write(buff + i, 1))
                parser_status_ = PARSE_ERROR;

            if (now_message_->get_body_len() >= content_len_cache_)
            {
                parser_status_ = PARSE_INIT;
                set_message(now_message_);
            }
            LOG_DBG << "body:" << content_len_cache_ << ":" << now_message_->get_body_len() << ",parser_status_ " << parser_status_;
        }
    }

    return parser_status_;
}

void msg::MessageParser::set_message(std::shared_ptr<Message> message)
{
    message_queue_.push(message);
}
