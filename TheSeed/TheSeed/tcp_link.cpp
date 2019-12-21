#include "tcp_link.h"

#include <mutex>

#include "logging.hpp"
#include <memory>
using namespace net::tcp;
net::tcp::TcpLink::TcpLink():buff_size(1024), time_out_s_(60),  timer_(nullptr)
{
}

std::shared_ptr<TcpLink> net::tcp::TcpLink::generate()
{
    return std::make_shared<TcpLink>();
}

net::tcp::TcpLink::~TcpLink()
{
    LOG_DBG << "tcp link is disstruct " << get_remote_ip() << ":" << get_remote_port();
}

bool net::tcp::TcpLink::set_on_connect(ON_CONNECT handler)
{
    on_connect_ = handler;
    return on_connect_.operator bool();
}

bool net::tcp::TcpLink::set_on_message(ON_MESSAGE handler)
{
    on_message_ = handler;
    return on_message_.operator bool();
}

bool net::tcp::TcpLink::set_on_close(ON_CLOSE handler)
{
    on_close_ = handler;
    return on_close_.operator bool();
}

bool net::tcp::TcpLink::bind(socket_ptr sock)
{
    sock_ = sock;
    return nullptr == sock;
}

bool net::tcp::TcpLink::connect(io_service& ios, std::string ip, unsigned int port)
{
    sock_ = std::make_shared<tcp::socket>(ios);
    
    boost::asio::ip::tcp::resolver resolver(ios);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(ip, std::to_string(port)).begin();
    boost::system::error_code ec;
    tcp_set_timer(time_out_s_);
    //读到数据后的回调函数
    auto connect_handler = [this](boost::system::error_code ec)
    {
        if (ec)
        {
            LOG_ERR << "连接失败 " << ec.value() << " " << ec.message();
            default_on_connect(false);
            return;
        }
        default_on_connect(true);
    };

    sock_->async_connect(endpoint, connect_handler);
    return true;
}

bool net::tcp::TcpLink::start(const std::string& message)
{
    if (is_active())
    {
        tcp_set_timer(time_out_s_);
        async_send(message);
        async_recv();
        return true;
    }
    return false;
}

bool net::tcp::TcpLink::close()
{
    try
    {
        if (is_active())
        {
            sock_->shutdown(boost::asio::socket_base::shutdown_both);
            sock_->close();
            default_on_close(true);
        }
        return false;
    }
    catch (std::exception & e)
    {
        return false;
    }
}

void net::tcp::TcpLink::async_recv()
{
    
    //申请buff
    std::shared_ptr<char> buff_ptr;
    try
    {
        buff_ptr = SHARED_BUFF_PTR(buff_size);
    }
    catch (std::bad_alloc & e)
    {

        LOG_ERR << "申请接收缓存区失败 what=" << e.what()<<",buff_size="<< buff_size;
        default_on_close(false);
        return;
    }
    //读到数据后的回调函数
    auto read_handler = \
        [this, buff_ptr](boost::system::error_code ec, std::size_t s)mutable
    {

        if (ec)
        {
            LOG_ERR << "读数据错误:" << ec.value() << " " << ec.message();
            default_on_close(false);
            return;
        }
        if (buff_ptr && s > 0)
        {
            //处理数据
            std::string req(buff_ptr.get(), s);
            std::string res;
            auto keep_alive = default_on_message(req, res);
            if (!res.empty())
            {
                async_send(res);
            }
            if (keep_alive)
            {
                async_recv();
            }
            else
            {
                default_on_close(false);
            }
            return;
        }
        return;
    };

    //调用接口，从异步网络读取数据，一有数据就返回
    sock_->async_read_some(boost::asio::buffer(buff_ptr.get(), buff_size), read_handler);
}

void net::tcp::TcpLink::async_send(const std::string& message)
{
    if (message.empty())
    {
        return;
    }
    std::shared_ptr<char>buff(new char[message.size() + 10]{ 0 }, std::default_delete<char[]>());
    memcpy(buff.get(), message.c_str(), message.size());
    async_send(buff, message.size(),0);
}

void net::tcp::TcpLink::async_send(std::shared_ptr<char> buff_ptr, size_t len, size_t begin)
{
    //写完数据后的回调函数
    auto write_handler = \
        [this, len, begin, buff_ptr](boost::system::error_code ec, std::size_t s)
    {

        if (ec)
        {
            LOG_ERR << "写数据错误:" << ec.value() << " " << ec.message();
            default_on_close(false);
            return;
        }
        //写完了
        if (len <= s)
        {
            //返回异步写,取下一个buff
           // if (on_complate_)
            //    on_complate_(true);
            return;
        }
        //未写完
        //计算剩下的字节
        int now_len = len - s;
        //继续写
        async_send( buff_ptr, now_len, begin + s);
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
   
    try
    {
        if (sock_)
            return sock_->remote_endpoint().address().to_string();
        return std::string();
    }
    catch (std::exception & e)
    {
        return std::string();
    }
}

int net::tcp::TcpLink::get_remote_port()
{
    try
    {
        if (sock_)
            return sock_->remote_endpoint().port();
        return -1;
    }
    catch (std::exception & e)
    {
        return -1;
    }
    
}

std::string net::tcp::TcpLink::get_local_ip()
{
    try
    {
        if (sock_)
            return sock_->local_endpoint().address().to_string();
        return "";
    }
    catch (std::exception & e)
    {
        return std::string();
    }
}

int net::tcp::TcpLink::get_local_port()
{
    try
    {
        if (sock_)
            return sock_->local_endpoint().port();
        return -1;
    }
    catch (std::exception & e)
    {
        return -1;
    }
}

bool net::tcp::TcpLink::is_active()
{
    if (sock_)
    {
        try
        {
            if (sock_->is_open())
            {
                return true;
            }
        }
        catch (std::exception & e)
        {
            return false;
        }
    }
    return false;
}

void net::tcp::TcpLink::default_on_close(bool succeed)
{
    auto l = shared_from_this();
    {
        std::lock_guard<std::mutex> lk(call_lock_);

        if (on_close_)
            on_close_(false);
        on_connect_ = nullptr;
        on_message_ = nullptr;
        on_close_ = nullptr;
    }

    //close();
}

void net::tcp::TcpLink::default_on_connect(bool succeed)
{
    std::lock_guard<std::mutex> lk(call_lock_);
    tcp_set_timer(time_out_s_);
    if (on_connect_)
        on_connect_(succeed);
}

bool net::tcp::TcpLink::default_on_message(std::string& in, std::string& out)
{
    std::lock_guard<std::mutex> lk(call_lock_);
    tcp_set_timer(time_out_s_);
    if (on_message_)
        return on_message_(in, out);
    else
        return false;
}

bool net::tcp::TcpLink::tcp_set_timer(int time_out)
{
    tcp_clear_timer();
    timer_ = start_timer(time_out, [this]() 
        {
            close();
        });
    return nullptr == timer_;
}

bool net::tcp::TcpLink::tcp_clear_timer()
{
    if(nullptr == timer_)
        return false;
    auto ret = stop_timer(timer_);
    timer_ = nullptr;
    return 0 == ret;
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
