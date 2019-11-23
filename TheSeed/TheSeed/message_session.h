/*
    数据传输会话
*/
#ifndef TRANSMISSION_SESSION_H_
#define TRANSMISSION_SESSION_H_
#include "tcp_server.h"
#include "message_parser.h"
namespace msg
{
    //会话
    //每个请求都有 Type 字段判断 消息类型 请求都有 token字段  返回是对应某个请求

    //请求句柄
    typedef std::function<void(std::shared_ptr<Message> message, int error_code)> ReqHandler;
    typedef std::function<void(int error_code)> SendHandler;

    class MessageSession
    {
        MessageSession();
    public:
        ~MessageSession();

       static  std::shared_ptr<MessageSession> create_sesson(net::tcp::TcpLink::PTcpLink link);

        bool recv_message(ReqHandler req_handler);

        //发送请求
        bool send_message(std::shared_ptr<Message> message, SendHandler handler);

    private:
        bool send_head(std::shared_ptr<Message> message, SendHandler handler);

        bool send_body(std::shared_ptr<Message> message, SendHandler handler, std::shared_ptr<char> buff, int buff_len);

        //接收完数据 返回
        void on_recv(ReqHandler req_handler,std::shared_ptr<char> recv_data, int recv_len, int err_code);
    private:
        MessageParser parser_;
        //连接
        net::tcp::TcpLink::PTcpLink link_;
    };
}
#endif