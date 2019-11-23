#include "test.h"
#include <chrono>
#include <thread>
//typedef std::function<void(std::shared_ptr<Message> request,int err)> Handler;
using namespace msg;

void send_messages(std::shared_ptr<Message> request)
{
    
    msg::MessageBus::instance().send_message\
        (request, [request](std::shared_ptr<Message> request, int err)
            {
                if (0 == err)
                {
                    auto l = request->get_body_len();
                    auto buff = util::make_shared_buff(l + 10);
                    request->read_body(buff.get(), l);
                    LOG_INFO << "recv a response :" << buff.get();
                    send_messages(request);
                }
                else
                {
                    LOG_ERR << " send message error,code=" << err;
                }
            });
}
int test_data_transmission()
{
    auto& thiz = msg::MessageBus::instance();

    thiz.start();

    thiz.set_cmd_cb("TEST", [](std::shared_ptr<Message> request, int err) {
        if (0 == err)
        {
            auto l = request->get_body_len();
            auto buff = util::make_shared_buff(l + 10);
            request->read_body(buff.get(), l);
            LOG_INFO << "recv a TEST message:" << buff.get();

            //·µ»Ø
            auto response = std::make_shared<Message>();
            auto dst = request->get_src_node();

            response->set_method(METHOD_Response);
            response->set_token(request->get_token());
            response->set_dst_node(dst.first, dst.second);
            std::string msg = "hello word";
            response->write_body(msg.c_str(), msg.size());

            msg::MessageBus::instance().send_message\
                (response, [](std::shared_ptr<Message> request, int err)
                {
                if (1 == err)
                {
                    LOG_ERR << " send response ok";
                }
                else
                {
                    LOG_ERR << " send response error,code=" << err;
                }
                });
        }
        else
        {
            LOG_ERR << " recv message error,code=" << err;
        }
        });

    //·µ»Ø
    auto request = std::make_shared<Message>();
    request->set_method(METHOD_Request);
    request->set_cmd("TEST");
    request->set_dst_node("192.168.1.100", default_port);
    std::string msg = "hello word";
    request->write_body(msg.c_str(), msg.size());
    while (true)
    {
        send_messages(request);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return 0;
}
