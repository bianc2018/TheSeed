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
            //无效的句柄 invalid
            TCP_ERROR_CODE_INVALID_HANDLE = -1,

            TCP_ERROR_CODE_RECV_ERROR = -2,
            TCP_ERROR_CODE_RECV_TIME_OUT = -3,

            TCP_ERROR_CODE_SEND_ERROR = -4,
            TCP_ERROR_CODE_SEND_TIME_OUT = -5,

            //客户端 连接失败
            TCP_ERROR_CLIENT_CONNECT_ERROR = -6,
            //连接超时
            TCP_ERROR_CLIENT_CONNECT_TIME_OUT = -7,
            //服务端 开启失败
            TCP_ERROR_SERVER_OPEN_ERROR = -8,
            //服务端 set_option 设置参数错误
            TCP_ERROR_SERVER_SET_ERROR = -9,
            //服务端 绑定地址失败
            TCP_ERROR_SERVER_BIND_ERROR = -10,

            //连接句柄无效
            TCP_ERROR_LINK_HANDLE_ERROR = -10,
            //服务句柄无效
            TCP_ERROR_SERVER_HANDLE_ERROR = -11,
            TCP_ERROR_ACCEPT_ERROR = -12,
            //申请缓存区失败
            TCP_ERROR_RECV_BUFF_NEW_ERROR = -13,
        };

        typedef boost::asio::ip::tcp::socket socket;
        typedef std::shared_ptr<tcp::socket>  socket_ptr;
        //服务
        typedef boost::asio::io_context io_service;

        //连接，每一个对象实例代表一个连接实例
		class TcpLink: public std::enable_shared_from_this<TcpLink>
		{
        public:
            typedef std::shared_ptr<TcpLink>  PTcpLink;
            //接收完数据 返回
            typedef  std::function<void(std::shared_ptr<char> recv_data, int recv_len, int err_code)> RECV_HANDLER;
            //发送完数据
            typedef  std::function<void(int err_code)> SEND_HANDLER;
            //连接句柄
            typedef std::function<void(PTcpLink link, int err_code)>  CONNECT_HANDLER;

            //初始化，传入接收到的连接socket
            TcpLink();
		public:
			
			//解析
			~TcpLink();

            static PTcpLink connect(socket_ptr sock);

            static PTcpLink connect(io_service&ios,std::string ip,unsigned int port);

            static void async_connect(io_service& ios, CONNECT_HANDLER handler,
                std::string ip, unsigned int port,  int time_out=60);

			//异步接收数据
			void async_recv(RECV_HANDLER recv_handler, int buff_size= FRAME_SIZE, int time_out = 60);
			
            //异步发送数据
			void async_send(SEND_HANDLER send_handler, std::shared_ptr<char> buff_ptr, size_t len, size_t begin=0, int time_out=60);

			//获取连接对应的套接字
			std::shared_ptr<boost::asio::ip::tcp::socket> get_sock_ptr();

            std::string get_remote_ip();

            int get_remote_port();

            std::string get_local_ip();

            int get_local_port();

            bool is_active();
		public:
			//启动一个计时器
			std::shared_ptr<boost::asio::steady_timer> start_timer(int time_out, boost::function<void()> call);
			//关闭一个计时器
			int stop_timer(std::shared_ptr<boost::asio::steady_timer>);
		private:
			
			//套接字
            socket_ptr sock_;
		};
	}
}
#endif //
