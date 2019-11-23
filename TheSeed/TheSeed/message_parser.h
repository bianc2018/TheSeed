/*
    消息解析器
    输入数据 然后 输出message
*/
#ifndef MESSAGE_PARSER_H_
#define MESSAGE_PARSER_H_
#include <queue>

#include "message.h"

namespace msg
{
    //class BuffCache
    enum ParserStatus
    {
        PARSE_INIT = 0,
        PARSE_HEAD_KEY = 1,
        PARSE_HEAD_VALUE = 2,
        PARSE_BODY = 3,
        PARSE_ERROR = -1,
    };

    class MessageParser
    {
    public:
        MessageParser();

        //数据
        std::shared_ptr<Message> get_message();

        //解析
        ParserStatus parse(const char* buff, size_t len);
    private:
        void set_message(std::shared_ptr<Message> message);
    private:
        //解析状态
        ParserStatus parser_status_;
        //解析缓存
        std::string parser_cache1_;
        std::string parser_cache2_;

        size_t content_len_cache_;

        std::shared_ptr<Message> now_message_;

        std::queue<std::shared_ptr<Message>> message_queue_;
    };
}

#endif