//�첽��tcp������

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
		//����� p ip::tcp::v4()
		#define ipv4 boost::asio::ip::tcp::v4()
		
		//�׽���
		typedef boost::asio::ip::tcp::socket sock;
		typedef std::shared_ptr<sock> sock_ptr;
		//�˵�
		typedef boost::asio::ip::tcp::endpoint point;
		//��ַ ::address
		typedef boost::asio::ip::address address;
		//���� acceptor
		typedef boost::asio::ip::tcp::acceptor acceptor;

        typedef boost::function<void(TcpLink::PTcpLink link, int err)> ACCEPT_HANDLER;
		//��ʾһ��������
		class TcpServer :public boost::noncopyable
		{
		public:
			TcpServer(io_service &service);
			
            bool start(unsigned int port, ACCEPT_HANDLER handler,int accept_num = 2);
			
            bool stop();

			~TcpServer();
		private:
			void accept(int accept_num);
            void accept_event();
		private:
			
			//�첽����
			io_service &service_;

			//��������
			acceptor server_;

            ACCEPT_HANDLER handler_;

            std::atomic_bool run_flag_;
		};
	}
}
#endif