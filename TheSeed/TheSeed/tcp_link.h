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
        enum TCP_ERROR_CODE
        {
            TCP_ERROR_CODE_OK = 0,
            //��Ч�ľ�� invalid
            TCP_ERROR_CODE_INVALID_HANDLE = -1,

            TCP_ERROR_CODE_RECV_ERROR = -2,
            TCP_ERROR_CODE_RECV_TIME_OUT = -3,

            TCP_ERROR_CODE_SEND_ERROR = -4,
            TCP_ERROR_CODE_SEND_TIME_OUT = -5,

            //�ͻ��� ����ʧ��
            TCP_ERROR_CLIENT_CONNECT_ERROR = -6,
            //���ӳ�ʱ
            TCP_ERROR_CLIENT_CONNECT_TIME_OUT = -7,
            //����� ����ʧ��
            TCP_ERROR_SERVER_OPEN_ERROR = -8,
            //����� set_option ���ò�������
            TCP_ERROR_SERVER_SET_ERROR = -9,
            //����� �󶨵�ַʧ��
            TCP_ERROR_SERVER_BIND_ERROR = -10,

            //���Ӿ����Ч
            TCP_ERROR_LINK_HANDLE_ERROR = -10,
            //��������Ч
            TCP_ERROR_SERVER_HANDLE_ERROR = -11,
            TCP_ERROR_ACCEPT_ERROR = -12,
            //���뻺����ʧ��
            TCP_ERROR_RECV_BUFF_NEW_ERROR = -13,
        };

        typedef boost::asio::ip::tcp::socket socket;
        typedef std::shared_ptr<tcp::socket>  socket_ptr;
        //����
        typedef boost::asio::io_context io_service;

        //���ӣ�ÿһ������ʵ������һ������ʵ��
		class TcpLink: public std::enable_shared_from_this<TcpLink>
		{
        public:
            typedef std::shared_ptr<TcpLink>  PTcpLink;
            //���������� ����
            typedef  std::function<void(std::shared_ptr<char> recv_data, int recv_len, int err_code)> RECV_HANDLER;
            //����������
            typedef  std::function<void(int err_code)> SEND_HANDLER;
            //���Ӿ��
            typedef std::function<void(PTcpLink link, int err_code)>  CONNECT_HANDLER;

            //��ʼ����������յ�������socket
            TcpLink();
		public:
			
			//����
			~TcpLink();

            static PTcpLink connect(socket_ptr sock);

            static PTcpLink connect(io_service&ios,std::string ip,unsigned int port);

            static void async_connect(io_service& ios, CONNECT_HANDLER handler,
                std::string ip, unsigned int port,  int time_out=60);

			//�첽��������
			void async_recv(RECV_HANDLER recv_handler, int buff_size= FRAME_SIZE, int time_out = 60);
			
            //�첽��������
			void async_send(SEND_HANDLER send_handler, std::shared_ptr<char> buff_ptr, size_t len, size_t begin=0, int time_out=60);

			//��ȡ���Ӷ�Ӧ���׽���
			std::shared_ptr<boost::asio::ip::tcp::socket> get_sock_ptr();

            std::string get_remote_ip();

            int get_remote_port();

            std::string get_local_ip();

            int get_local_port();

            bool is_active();
		public:
			//����һ����ʱ��
			std::shared_ptr<boost::asio::steady_timer> start_timer(int time_out, boost::function<void()> call);
			//�ر�һ����ʱ��
			int stop_timer(std::shared_ptr<boost::asio::steady_timer>);
		private:
			
			//�׽���
            socket_ptr sock_;
		};
	}
}
#endif //
