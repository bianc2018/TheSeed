#include "tcp_service.h"

#include "src/tcp_service_impl.h"
#include "log/log.hpp"

void net_service::tcp::async_recv(TCP_HANDLE handle, RECV_HANDLER recv_handler, int time_out)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.async_recv(handle,recv_handler,time_out);
}

void net_service::tcp::async_send(TCP_HANDLE handle, std::shared_ptr<char> data, int len, SEND_HANDLER send_handler, int time_out)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.async_send(handle, data, len, send_handler, time_out);
}

void net_service::tcp::run()
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.run_service();
}

TCP_SERVICE_API void net_service::tcp::post_task(TASK_HANDLER task)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.post_task(task);
}

ADDRESSS net_service::tcp::get_remote_address(TCP_HANDLE handle)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	return impl.get_remote_address(handle);
}

ADDRESSS net_service::tcp::get_local_address(TCP_HANDLE handle)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	return impl.get_local_address(handle);
}

TCP_HANDLE_VEC net_service::tcp::get_links_handle(TCP_HANDLE server_handle)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	return impl.get_links_handle(server_handle);
}

TCP_HANDLE net_service::tcp::get_server_handle(TCP_HANDLE link_handle)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	return impl.get_server_handle(link_handle);
}

void net_service::tcp::close_link(TCP_HANDLE handle)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.close_client(handle);
}

void net_service::tcp::close_server(TCP_HANDLE handle)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.close_server(handle);
}

int net_service::tcp::set_recv_buff(size_t value)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.set_buff_size(value);
	return 0;
}

int net_service::tcp::set_log_path(const std::string & path)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.set_log_path(path);
	return 0;
}

int net_service::tcp::set_thread_num(size_t value)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.set_thread_num(value);
	return 0;
}

TCP_SERVICE_API int net_service::tcp::set_log_lv(int lv)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	impl.set_log_lv(lv);
	return 0;
}

TCP_HANDLE net_service::tcp::start_server(const std::string & ip, int port, ACCEPT_HANDLER accept_handler, int accept_num)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	return impl.start_server(ip, port, accept_handler, accept_num);
}

TCP_HANDLE net_service::tcp::start_client(const std::string & ip, int port)
{
	net_service::tcp::TcpServiceImpl& impl = net_service::tcp::TcpServiceImpl::instance();
	return impl.start_client(ip, port);
}
