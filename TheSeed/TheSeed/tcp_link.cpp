#include "tcp_link.h"

#include <mutex>

#include "logging.hpp"
using namespace net::tcp;
net::tcp::TcpLink::TcpLink()
{
}

net::tcp::TcpLink::~TcpLink()
{
	//关闭套接字
	boost::system::error_code ec;
	sock_->cancel(ec);
	sock_->close(ec);

}

TcpLink::PTcpLink net::tcp::TcpLink::connect(socket_ptr sock)
{
    if (nullptr == sock)
        return nullptr;

    TcpLink::PTcpLink ptr (new TcpLink);
    ptr->sock_ = sock;
    return ptr;
}

TcpLink::PTcpLink net::tcp::TcpLink::connect(io_service& ios, std::string ip, unsigned int port)
{
    std::shared_ptr<tcp::socket> sock = std::make_shared<tcp::socket>(ios);

    boost::asio::ip::tcp::resolver resolver(ios);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(ip, std::to_string(port)).begin();
    boost::system::error_code ec;

    sock->connect(endpoint, ec);
    if (ec)
    {
        LOG_ERR << "连接失败 " << ec.value() << " " << ec.message();
        return nullptr;
    }
    return connect(sock);
}

void net::tcp::TcpLink::async_connect(io_service& ios, CONNECT_HANDLER handler, std::string ip, unsigned int port, int time_out)
{
    auto ptr = std::make_shared<TcpLink>();

    std::shared_ptr<tcp::socket> sock = std::make_shared<tcp::socket>(ios);
    ptr->sock_ = sock;
    boost::asio::ip::tcp::resolver resolver(ios);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(ip, std::to_string(port)).begin();
    boost::system::error_code ec;

    auto timer = ptr->start_timer(time_out, boost::bind(handler, ptr, TCP_ERROR_CLIENT_CONNECT_TIME_OUT));
    
    //读到数据后的回调函数
    auto connect_handler = [handler, timer,ptr](boost::system::error_code ec)
    {
        ptr->stop_timer(timer);

        if (ec)
        {
            LOG_ERR << "连接失败 " << ec.value() << " " << ec.message();
            handler(ptr, TCP_ERROR_CLIENT_CONNECT_ERROR);
            return ;
        }
        handler(ptr, TCP_ERROR_CODE_OK);
    };

    sock->async_connect(endpoint, connect_handler);
    
}



void net::tcp::TcpLink::async_recv(RECV_HANDLER recv_handler,int buff_size, int time_out)
{
	//申请buff
	std::shared_ptr<char> buff_ptr;
	try
	{
		buff_ptr = SHARED_BUFF_PTR(buff_size);
	}
	catch (std::bad_alloc &e)
	{

        LOG_ERR << "申请接收缓存区失败 what="<<e.what();
		recv_handler(nullptr,0, TCP_ERROR_RECV_BUFF_NEW_ERROR);
		return;
	}
	//申请 读buff
	//auto buff_ptr = SHARED_BUFF_PTR(buff_size);
	//开始计时
	auto timer = start_timer(time_out, boost::bind(recv_handler, nullptr, 0, TCP_ERROR_CODE_RECV_TIME_OUT));
	//读到数据后的回调函数
	auto read_handler = \
		[this,buff_ptr, recv_handler, timer](boost::system::error_code ec, std::size_t s)mutable
	{
		stop_timer(timer);

		if (ec)
		{
            LOG_ERR << "读数据错误:"<<ec.value()<<" "<<ec.message();
			recv_handler( nullptr, 0, TCP_ERROR_CODE_RECV_ERROR);
			return;
		}
		//必须重新创建变量，不然数据无法传递给after_async_read 原理未明
		//auto n_handle_ = handle_;
		//auto n_buff_ptr_ = buff_ptr;
		//auto n_size_ = s;
		//处理数据
		recv_handler(buff_ptr, s,TCP_ERROR_CODE_OK);
		return;
	};
	
	//调用接口，从异步网络读取数据，一有数据就返回
	sock_->async_read_some(boost::asio::buffer(buff_ptr.get(), buff_size), read_handler);
	
}

void net::tcp::TcpLink::async_send(SEND_HANDLER send_handler, std::shared_ptr<char> buff_ptr, size_t len, size_t begin, int time_out)
{
	//开始计时
	auto timer = start_timer(time_out, boost::bind(send_handler, TCP_ERROR_CODE_SEND_TIME_OUT));
	//写完数据后的回调函数
	auto write_handler = \
		[this,len, begin, buff_ptr, send_handler,time_out,timer](boost::system::error_code ec, std::size_t s)
	{
		stop_timer(timer);

		if (ec)
		{
            LOG_ERR << "写数据错误:" << ec.value() << " " << ec.message();
			send_handler(TCP_ERROR_CODE_SEND_ERROR);
			return;
		}
		//写完了
		if (len <= s)
		{
			//返回异步写,取下一个buff
			send_handler(TCP_ERROR_CODE_OK);
			return;
		}
		//未写完
		//计算剩下的字节
		int now_len = len - s;
		//继续写
		async_send(send_handler, buff_ptr, now_len, begin + s, time_out);
		return;
	};

	sock_->async_write_some(boost::asio::buffer(buff_ptr.get() + begin, len), write_handler);
}

std::shared_ptr<boost::asio::ip::tcp::socket> net::tcp::TcpLink::get_sock_ptr()
{
	return sock_;
}

std::string net::tcp::TcpLink::get_remote_ip()
{
    if(sock_)
        return sock_->remote_endpoint().address().to_string();
    return std::string();
}

int net::tcp::TcpLink::get_remote_port()
{
    if (sock_)
        return sock_->remote_endpoint().port();
    return -1;
}

std::string net::tcp::TcpLink::get_local_ip()
{
    if (sock_)
        return sock_->local_endpoint().address().to_string();
    return "";
}

int net::tcp::TcpLink::get_local_port()
{
    if (sock_)
        return sock_->local_endpoint().port();
    return -1;
}

bool net::tcp::TcpLink::is_active()
{
    if (sock_)
    {
        if (sock_->is_open())
        {
            return true;
        }
    }
    return false;
}


std::shared_ptr<boost::asio::steady_timer> net::tcp::TcpLink::start_timer(int time_out, boost::function<void()> call)
{

	//定时器
	auto timer = std::make_shared<boost::asio::steady_timer>(sock_->get_io_context());
	
	//设置超时
	auto time_out_handler = \
		[this, call](boost::system::error_code ec)
	{
		//LOG(LWARN, "Tcp Link timer 超时,handle_=", handle_,ec.value(),ec.message());
		if (!ec)
		{
            LOG_WARN << "Tcp Link timer 超时";
			call();
		}
	};
	
	timer->expires_from_now(std::chrono::seconds(time_out));
	timer->async_wait(time_out_handler);
	return timer;
}

int net::tcp::TcpLink::stop_timer(std::shared_ptr<boost::asio::steady_timer> timer)
{
	boost::system::error_code ec;
	timer->cancel(ec);
	return ec.value();
}
