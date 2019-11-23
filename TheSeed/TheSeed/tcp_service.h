#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

#include <string>

#include "tcp_define.h"
namespace net_service
{
	namespace tcp
	{
		//开启一个服务器
		TCP_SERVICE_API TCP_HANDLE start_server(const std::string &ip, int port, ACCEPT_HANDLER accept_handler, int accept_num = 4);

		//连接 返回连接句柄 客户端
		TCP_SERVICE_API TCP_HANDLE start_client(const std::string &ip, int port);

		//接收数据
		TCP_SERVICE_API void async_recv(TCP_HANDLE handle, RECV_HANDLER recv_handler, int time_out);

		//接收数据
		TCP_SERVICE_API void async_send(TCP_HANDLE handle, std::shared_ptr<char> data, int len, SEND_HANDLER send_handler, int time_out);

		TCP_SERVICE_API void run();
		//提交任务
		TCP_SERVICE_API void post_task(TASK_HANDLER task);
		//获取远程用户的ip和port
		TCP_SERVICE_API ADDRESSS get_remote_address(TCP_HANDLE handle);
		//获取本地端的ip和port
		TCP_SERVICE_API ADDRESSS get_local_address(TCP_HANDLE handle);

		//根据服务器句柄server_handle 获取其下的连接 p_handle p_handle_len要获取的数目 all 全部的连接数目 返回 时间获取到数目
		TCP_SERVICE_API TCP_HANDLE_VEC get_links_handle(TCP_HANDLE server_handle);
		//根据连接句柄 获取其属的服务器
		TCP_SERVICE_API TCP_HANDLE get_server_handle(TCP_HANDLE link_handle);

		//关闭一个连接 服务器
		TCP_SERVICE_API void close_link(TCP_HANDLE handle);
		TCP_SERVICE_API void close_server(TCP_HANDLE handle);

		//设置参数
		TCP_SERVICE_API int set_recv_buff(size_t value);
		TCP_SERVICE_API int set_log_path(const std::string &path);
		TCP_SERVICE_API int set_thread_num(size_t value);
		TCP_SERVICE_API int set_log_lv(int lv);
	
	}
}

#endif // !TCP_SERVICE_H


