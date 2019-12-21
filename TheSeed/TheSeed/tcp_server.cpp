#include "tcp_server.h"

#include <mutex>

#include "ns_event.hpp"
#include "logging.hpp"

net::tcp::TcpServer::TcpServer(io_service & service):\
	service_(service),server_(service_), run_flag_(true)
{
	//LOG(LINFO, "initing server：ip=", ip, ":", port);

	

}
bool net::tcp::TcpServer::set_on_accept(ON_ACCEPT fn)
{
    on_accept_ = fn;
    return true;
}
bool net::tcp::TcpServer::start(unsigned int port, int accept_num)
{
    boost::asio::ip::tcp::resolver resolver(service_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve("0.0.0.0", std::to_string(port)).begin();

    boost::system::error_code ec;
    //打开连接
    server_.open(endpoint.protocol(), ec);
    if (ec)
    {
        LOG_ERR << "open error " << ec.value() << ec.message();
        return false;
    }
    //设置参数，地址可重用
    server_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    if (ec)
    {
        LOG_ERR << "set_option reuse_address error " << ec.value() << ec.message();
        return false;
    }
    //绑定地址
    server_.bind(endpoint, ec);
    if (ec)
    {
        LOG_ERR << "bind error " << ec.value() << ec.message();
        return false;
    }

    //监听
    server_.listen(boost::asio::socket_base::max_listen_connections,ec);
    if (ec)
    {
        LOG_ERR << "listen error " << ec.value() << ec.message();
        return false;
    }
    //接收
    run_flag_ = true;
    accept(accept_num);
    return true;
}
net::tcp::TcpServer::~TcpServer()
{
    stop();
}

void net::tcp::TcpServer::accept(int accept_num)
{

    for (int i = 0; i < accept_num; ++i)
    {
        accept_event();
    }
}

void net::tcp::TcpServer::accept_event()
{
    if (run_flag_)
    {
        //创建客户端套接字
        sock_ptr client(new sock(service_));

        //异步接收连接
        server_.async_accept(*(client), [this, client](const boost::system::error_code& error)
            {
                auto link = tcp::TcpLink::generate();
                link->bind(client);
                if (error)
                {
                    LOG_ERR << error.value() <<" "<< error.message();
                }
                else
                {
                    if (client->is_open())
                    {
                        LOG_INFO << "获得连接 " << link->get_remote_ip() << ":" << link->get_remote_port();
                        //协议栈异步处理连接
                        if(on_accept_)
                            on_accept_(link);

                    }
                    else
                    {
                        LOG_INFO << "获得连接：套接字错误 " ;
                        //协议栈异步处理连接

                    }
                    //重复接收
                    accept_event();

                }

            });

    }
}

bool net::tcp::TcpServer::stop()
{
    run_flag_ = false;
	//退出系统
	boost::system::error_code ec;
	server_.cancel(ec);
	server_.close(ec);

	return true;
}

