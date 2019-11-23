#include "message_session.h"

#include <functional>

#include "logging.hpp"
#include "ns_event.hpp"
#include "util.hpp"
using namespace msg;
msg::MessageSession::MessageSession()
{
}
msg::MessageSession::~MessageSession()
{
}

std::shared_ptr<MessageSession> msg::MessageSession::create_sesson(net::tcp::TcpLink::PTcpLink link)
{
    if (link)
    {
        if (link->is_active())
        {
            std::shared_ptr<MessageSession> ptr (new MessageSession);
            ptr->link_ = link;
           
            return ptr;
        }
    }
    return nullptr;
}

bool msg::MessageSession::recv_message(ReqHandler req_handler)
{
    link_->async_recv(std::bind(&MessageSession::on_recv, this, req_handler, \
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    return true;

}

bool msg::MessageSession::send_message(std::shared_ptr<Message> message, SendHandler handler)
{
    if (message)
        return send_head(message, handler);
    else
        return false;
}

bool msg::MessageSession::send_head(std::shared_ptr<Message> message, SendHandler handler)
{
    auto head = message->get_head();
    auto phead = util::make_shared_from_str(head);
    if (nullptr == phead)
        return false;
    link_->async_send([this, message,handler](int err_code)mutable {
        if (0 == err_code)
        {
            auto buff_len = FRAME_SIZE;
            auto buff = util::make_shared_buff(buff_len);
            if (nullptr == buff)
            {
                LOG_ERR << "make_shared_buff error" << buff_len;
                return;
            }
            send_body(message, handler, buff, buff_len);
        }
        else
        {
            LOG_ERR << "send_head error,code=" << err_code;
            if (handler)
            {
                NS_EVENT_ASYNC_VOID(handler, err_code);
            }

        }
        }, phead, head.size());
    return true;

}

bool msg::MessageSession::send_body(std::shared_ptr<Message> message, SendHandler handler,std::shared_ptr<char> buff,int buff_len)
{
    auto &body = message->get_body();
    auto size = body.read_content(buff.get(), buff_len);
    if (size<=0)
    {
        if (handler)
        {
            NS_EVENT_ASYNC_VOID(handler, 0);
        }
        return true;
    }
    link_->async_send([this, message, handler, buff, buff_len](int err_code)mutable {
        if (0 == err_code)
        {
            send_body(message, handler, buff, buff_len);
        }
        else
        {
            LOG_ERR << "send_head error,code=" << err_code;
            if (handler)
            {
                NS_EVENT_ASYNC_VOID(handler, err_code);
            }

        }
        }, buff, size);
    return true;
}

void msg::MessageSession::on_recv(ReqHandler req_handler, std::shared_ptr<char> recv_data, int recv_len, int err_code)
{
    if (0 == err_code)
    {
        auto err = parser_.parse(recv_data.get(), recv_len);
        if (PARSE_ERROR == err)
        {
            LOG_ERR << "message parser error";
            if (req_handler)
            {
                NS_EVENT_ASYNC_VOID(req_handler, nullptr, err_code);
            }
            return;
        }
        else
        {
            auto req = parser_.get_message();
            if (req)
            {
                NS_EVENT_ASYNC_VOID(req_handler, req, err_code);
            }

            recv_message(req_handler);
        }
    }
    else
    {
        LOG_ERR << "recv  date err_code=" << err_code;
        if (req_handler)
        {
            NS_EVENT_ASYNC_VOID(req_handler, nullptr, err_code);
        }

    }
}
