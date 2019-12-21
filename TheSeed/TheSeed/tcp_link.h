#ifndef TCP_LINK_H_
#define TCP_LINK_H_

#include <map>
#include <atomic>

#include <boost/bind.hpp>
#include "boost/function.hpp"
#include <boost/asio.hpp>

#include "util.hpp"
#define FRAME_SIZE 4096
namespace net
{
	namespace tcp
	{
        typedef boost::asio::ip::tcp::socket socket;

        typedef std::shared_ptr<tcp::socket>  socket_ptr;
        //服务
        typedef boost::asio::io_context io_service;

        //连接，每一个对象实例代表一个连接实例
        typedef std::function<bool(std::string & in, std::string & out)> ON_MESSAGE;

        //连接句柄
        typedef std::function<void(bool succeed)>  ON_CONNECT;

        //连接句柄
        typedef std::function<void(bool succeed)>   ON_CLOSE;

        class TcpLink :public std::enable_shared_from_this<TcpLink>
		{
            
        public:
            TcpLink();

            static std::shared_ptr<TcpLink> generate();

            //解析
            ~TcpLink();

            bool set_on_connect(ON_CONNECT handler);

            bool set_on_close(ON_CLOSE handler);

            bool set_on_message(ON_MESSAGE handler);

            bool bind(socket_ptr sock);

            bool connect(io_service& ios, std::string ip, unsigned int port);

            //启动流程
            bool start(const std::string& message="");

            bool close();
		public:
            void async_send(const std::string& message);

            void async_recv();

            void async_send(std::shared_ptr<char> buff_ptr, size_t len, size_t begin);

			//获取连接对应的套接字
			std::shared_ptr<boost::asio::ip::tcp::socket> get_sock_ptr();

            std::string get_remote_ip();

            int get_remote_port();

            std::string get_local_ip();

            int get_local_port();

            bool is_active();

            void default_on_close(bool succeed);
            void default_on_connect(bool succeed);
            bool default_on_message(std::string& in, std::string& out);

            bool tcp_set_timer(int time_out);

            bool tcp_clear_timer();
		public:
			
            //启动一个计时器
			std::shared_ptr<boost::asio::steady_timer> start_timer(int time_out, boost::function<void()> call);
			
            //关闭一个计时器
			int stop_timer(std::shared_ptr<boost::asio::steady_timer>);

        private:
			//套接字
            socket_ptr sock_;

            int buff_size;

            int time_out_s_;

            std::mutex call_lock_;

            ON_MESSAGE on_message_;

            ON_CLOSE on_close_;

            ON_CONNECT on_connect_;

            std::shared_ptr<boost::asio::steady_timer> timer_;

		};
	}
}
#endif //
