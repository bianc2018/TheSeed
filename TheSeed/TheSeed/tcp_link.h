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
        //����
        typedef boost::asio::io_context io_service;

        //���ӣ�ÿһ������ʵ������һ������ʵ��
        typedef std::function<bool(std::string & in, std::string & out)> ON_MESSAGE;

        //���Ӿ��
        typedef std::function<void(bool succeed)>  ON_CONNECT;

        //���Ӿ��
        typedef std::function<void(bool succeed)>   ON_CLOSE;

        class TcpLink :public std::enable_shared_from_this<TcpLink>
		{
            
        public:
            TcpLink();

            static std::shared_ptr<TcpLink> generate();

            //����
            ~TcpLink();

            bool set_on_connect(ON_CONNECT handler);

            bool set_on_close(ON_CLOSE handler);

            bool set_on_message(ON_MESSAGE handler);

            bool bind(socket_ptr sock);

            bool connect(io_service& ios, std::string ip, unsigned int port);

            //��������
            bool start(const std::string& message="");

            bool close();
		public:
            void async_send(const std::string& message);

            void async_recv();

            void async_send(std::shared_ptr<char> buff_ptr, size_t len, size_t begin);

			//��ȡ���Ӷ�Ӧ���׽���
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
			
            //����һ����ʱ��
			std::shared_ptr<boost::asio::steady_timer> start_timer(int time_out, boost::function<void()> call);
			
            //�ر�һ����ʱ��
			int stop_timer(std::shared_ptr<boost::asio::steady_timer>);

        private:
			//�׽���
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
