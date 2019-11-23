#include "message_bus.h"

#include <functional>

#include "logging.hpp"

#include "ns_event.hpp"

#include "util.hpp"


static net::tcp::io_service g_io_service;

msg::MessageBus::MessageBus():run_flag_(true), server_(g_io_service)
{
}

msg::MessageBus::~MessageBus()
{
    g_io_service.reset();
    run_flag_ = false;
    for (auto& t : run_thread_vec_)
    {
        if (t.joinable())
            t.join();
    }
}

msg::MessageBus& msg::MessageBus::instance()
{
    // TODO: 在此处插入 return 语句
    static MessageBus s_instance;
    return s_instance;
}

bool msg::MessageBus::start(int port, int thread_num)
{
    port_ =  port;
    if (false == server_.start(port_, \
        std::bind(&MessageBus::accept_cb, this, std::placeholders::_1, std::placeholders::_2), \
        thread_num / 2))
    {
        LOG_ERR << " tcp server start error";
        return false;
    }

    for (int i = 0; i < thread_num; ++i)
    {
        run_thread_vec_.push_back(std::thread([this]() 
            {
                while (run_flag_)
                {
                    g_io_service.run();
                }
            }
        ));
    }
    return true;
}

bool msg::MessageBus::set_cmd_cb(const std::string& cmd, Handler handler)
{
    cmd_handlers_[cmd] = handler;
    return true;
}

bool msg::MessageBus::send_message(std::shared_ptr<Message> message, Handler handler)
{
    if (message)
    {
        auto ip = message->get_head_value(DST_IP);
        auto port = message->get_head_value(DST_PORT, default_port);
        if ("" == ip)
        {
            LOG_ERR << "not find dst";
            return false;
        }

        auto method = message->get_head_value(METHOD, "");
        
        if (METHOD_Request == method)
        {
            auto token = util::uuid();
            message->set_head_value(TOKEN, token);
            if ("" != token)
            {
                if (!set_response_handler(token, handler))
                {
                    LOG_ERR << "set_response_handler token=" << token;
                    return false;
                }
            }
        }
        else if (METHOD_Response == method)
        {
            auto token = message->get_head_value(TOKEN);
            if ("" == token)
            {
                LOG_ERR << "not find token";
                return false;
            }
        }
        else
        {
            LOG_ERR << "nuknow method = " << method;
            return false;
        }
        net::tcp::TcpLink::async_connect(g_io_service, \
            [this, message, handler](net::tcp::TcpLink::PTcpLink link, int err_code)mutable
            {
                if (0 == err_code)
                {
                    auto session = MessageSession::create_sesson(link);
                    if (session)
                    {
                        message->set_head_value(SRC_IP, link->get_local_ip());
                        message->set_head_value(SRC_PORT, std::to_string(port_));

                        session->send_message(message, [this, message, session, handler](int error_code)mutable
                            {
                                auto token = message->get_head_value(TOKEN);
                                if (0 != error_code)
                                {
                                    LOG_ERR << "send_message[" << token << "] error,err_code=" << error_code;
                                    if (handler)
                                    {
                                        handler(nullptr, error_code);
                                    }


                                    if (token != "")
                                    {
                                        if(METHOD_Request == message->get_head_value(METHOD, ""))
                                            remove_response_handler(token);
                                    }
                                }
                                else
                                {
                                    LOG_INFO << "send message ok,token=" << token;
                                    handler(nullptr, 1);
                                }

                            });
                    }
                }
                else
                {
                    LOG_ERR << "async_connect error,err_code=" << err_code;
                    if (handler)
                    {
                        handler(nullptr, err_code);
                    }
                }
            }, \
            ip, port);
        return true;
    }
    LOG_ERR << "message is nullptr";
    return false;
}

void msg::MessageBus::accept_cb(net::tcp::TcpLink::PTcpLink link, int err)
{
    if (0 == err)
    {
        auto session = MessageSession::create_sesson(link);
        if (session)
        {
            //服务端只接收数据
            session->recv_message(std::bind(&MessageBus::recv_message,this,session,\
                std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            LOG_ERR << "create_sesson error ";
        }
    }
    else
    {
        LOG_ERR << "accept_cb error code =" << err;
    }
}

bool msg::MessageBus::set_response_handler(const std::string& token, Handler handler)
{
    auto p =  response_handlers_.find(token);
    if (response_handlers_.end() == p)
    {
        response_handlers_[token] = handler;
        return true;
    }
    return false;
}

bool msg::MessageBus::remove_response_handler(const std::string& token)
{
    response_handlers_.erase(token);
    return true;
}

void msg::MessageBus::recv_message(std::shared_ptr<MessageSession> session, std::shared_ptr<Message> message, int error_code)
{
    if (message)
    {
        NS_EVENT_ASYNC_VOID([this, message, error_code]()
            {
                auto method = message->get_head_value(METHOD);
                if ("" == method)
                {
                    LOG_ERR << "method is \"\"";
                    return;
                }
                if (METHOD_Request == method)
                {
                    call_cmd_handler(message, error_code);
                }
                else if (METHOD_Response == method)
                {
                    call_response_handler(message, error_code);
                }
                else
                {
                    LOG_ERR << "not find method=" << method;
                }
            });

        //不关闭
        if ("false" == message->get_head_value(Close, "true"))
        {
            session->recv_message(std::bind(&MessageBus::recv_message, this, session, \
                std::placeholders::_1, std::placeholders::_2));
        }
    }
    else
    {
        LOG_ERR << "recv message error,code="<<error_code;
    }
}

void msg::MessageBus::call_cmd_handler(std::shared_ptr<Message> request, int err)
{
    auto cmd = request->get_head_value(CMD);
    auto p = cmd_handlers_.find(cmd);
    if (cmd_handlers_.end() == p)
    {
        LOG_WARN << "cmd " << cmd << " is not find handler";
    }
    else
    {
        LOG_INFO << "get request,cmd = " << cmd;
        p->second(request, err);
    }
}

void msg::MessageBus::call_response_handler(std::shared_ptr<Message> request, int err)
{
    auto token = request->get_head_value(TOKEN);
    if ("" == token)
    {
        LOG_ERR << "token is \"\",not find the response  handler";
        return;
    }
    auto p = response_handlers_.find(token);
    if (response_handlers_.end() == p)
    {
        LOG_ERR << "token is "<< token<<",not find the response  handler";
        return;
    }
    p->second(request, err);
}
