//异步的tcp服务器

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <atomic>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/function.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include "tcp_link.h"

namespace net
{
	namespace tcp
	{
		//服务端 p ip::tcp::v4()
		#define ipv4 boost::asio::ip::tcp::v4()
		
		//套接字
		typedef boost::asio::ip::tcp::socket sock;
		typedef std::shared_ptr<sock> sock_ptr;
		//端点
		typedef boost::asio::ip::tcp::endpoint point;
		//地址 ::address
		typedef boost::asio::ip::address address;
		//接受 acceptor
		typedef boost::asio::ip::tcp::acceptor acceptor;

        typedef boost::function<void(std::shared_ptr<TcpLink>)> ON_ACCEPT;
		//表示一个服务器
		class TcpServer :public boost::noncopyable
		{
		public:
			TcpServer(io_service &service);
			
            bool set_on_accept(ON_ACCEPT fn);

            bool start(unsigned int port,int accept_num = 2);
			
            bool stop();

			~TcpServer();
		private:
			void accept(int accept_num);
            void accept_event();
		private:
			
			//异步服务
			io_service &service_;

			//接收连接
			acceptor server_;

            ON_ACCEPT on_accept_;

            std::atomic_bool run_flag_;
		};
	}
}
#endif
