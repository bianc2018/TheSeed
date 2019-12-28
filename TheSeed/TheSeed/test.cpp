#include "test.h"
#include <chrono>
#include <thread>
#include "tcp_server.h"
#include "logging.hpp"


/*测试tcp*/
using namespace net::tcp;
static net::tcp::io_service GIOS;
bool send_message(const std::string& msg,const std::string & ip,int port,std::function<void(bool ok)> fn)
{
    auto lk = TcpLink::generate();
    lk->set_on_message([lk,fn](std::string& in, std::string& out) {
        LOG_INFO << "recv msg:" << in << " from " << lk->get_remote_ip() << ":" << lk->get_remote_port();
        out = in;
        fn(true);
        return true;
        });
    lk->set_on_connect([lk,fn, msg](bool succeed)
        {
            if (succeed)
                lk->start(msg);
            else
            {
                LOG_ERR << "connect error";
                fn(false);
            }
        });
    lk->set_on_close([lk](bool su) {
        LOG_INFO << "link link on close " << lk.use_count()\
            << " endpoint " << lk->get_remote_ip() << ":" << lk->get_remote_port();
        lk->close();
        });
    return lk->connect(GIOS, ip, port);
}
time_t beg = 0;
bool send_message_complate(int i,bool ok)
{
    if (!ok)
        return false;

    LOG_INFO << "connect:" << ++i<<" ok "<<ok<<" used "<<time(nullptr)-beg<<" s";
    send_message("hello world", "127.0.0.1", 5060,
        [i](bool ok) mutable
        {
            send_message_complate(i, ok);

        });
    return true;
}

int test_tcp()
{
    net::tcp::TcpServer server(GIOS);
    server.set_on_accept([](std::shared_ptr<TcpLink> lk) {
        LOG_INFO << "get a link " << lk->get_remote_ip() << ":" << lk->get_remote_port();
        
        lk->set_on_message([lk](std::string& in, std::string& out) {
            LOG_INFO<<"recv msg:"<<in<<" from " << lk->get_remote_ip() << ":" << lk->get_remote_port();
            out = in;
            return true;
            });

        lk->set_on_close([lk](bool su) {
            LOG_INFO << "server link on close "<<lk.use_count()\
                <<" endpoint "<<lk->get_remote_ip()<<":"<< lk->get_remote_port();
            });

        lk->start();
        });

    beg = time(nullptr);
    int i = 0;
    server.start(5060);
    send_message("hello world","127.0.0.1", 5060, 
        [i](bool ok) mutable
        {
            //send_message_complate(i, ok);

        });

   // while(true)
        GIOS.run();
    return 0;
}

/*
    测试数据总线
*/
#include "message.hpp"
#include "message_bus.hpp"
typedef msg::Message<std::string> MYMSG;
bool on_request(std::shared_ptr<net::tcp::TcpLink> link, \
    std::shared_ptr<MYMSG> request, std::vector <std::shared_ptr<MYMSG>> responses)
{
    LOG_DBG << "request " << request->get_head().path \
        << " from " << link->get_remote_ip() << ":" << link->get_remote_port();
    if ("TRANSFER" == request->get_head().path)
    {
        std::string body;
        request->getbody(body);
        LOG_INFO << "TRANSFER " << body;
    }
    return false;
}
int test_message_bus()
{
    auto &bus = msg::MessageBus<MYMSG>::instance();
    bus.set_on_request(on_request);
    bus.start();

    auto request = std::make_shared<MYMSG>();
    request->get_head().path = "TRANSFER";
    request->get_head().dst = "127.0.0.1";
    request->setbody("hello world");
    bus.send_message(request);
    
    return 0;
}

#include "package_system.h"
int test_package_system()
{
    auto sys = package::PackageSystem::get_system();
    auto p = package::PackageSystem::get_basic_system();
    auto p1 = package::PackageSystem::get_basic_system();

    sys->scan_package("D:\\code\\TheSeed\\TheSeed\\bin\\Debug\\package\\");
    sys->run_instance("Test", "test1");
    sys->run_instance("Test", "test2");
    sys->run_instance("Test", "test3");
    system("pause");
    //sys->run_instance("Test", "test");
    //sys->run_instance("Test", "test1");
    //sys->stop_instance("test");
    
    sys->clear_all();
    return 0;
}
