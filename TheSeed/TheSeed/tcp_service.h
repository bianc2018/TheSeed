#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

#include <string>

#include "tcp_define.h"
namespace net_service
{
	namespace tcp
	{
		//����һ��������
		TCP_SERVICE_API TCP_HANDLE start_server(const std::string &ip, int port, ACCEPT_HANDLER accept_handler, int accept_num = 4);

		//���� �������Ӿ�� �ͻ���
		TCP_SERVICE_API TCP_HANDLE start_client(const std::string &ip, int port);

		//��������
		TCP_SERVICE_API void async_recv(TCP_HANDLE handle, RECV_HANDLER recv_handler, int time_out);

		//��������
		TCP_SERVICE_API void async_send(TCP_HANDLE handle, std::shared_ptr<char> data, int len, SEND_HANDLER send_handler, int time_out);

		TCP_SERVICE_API void run();
		//�ύ����
		TCP_SERVICE_API void post_task(TASK_HANDLER task);
		//��ȡԶ���û���ip��port
		TCP_SERVICE_API ADDRESSS get_remote_address(TCP_HANDLE handle);
		//��ȡ���ض˵�ip��port
		TCP_SERVICE_API ADDRESSS get_local_address(TCP_HANDLE handle);

		//���ݷ��������server_handle ��ȡ���µ����� p_handle p_handle_lenҪ��ȡ����Ŀ all ȫ����������Ŀ ���� ʱ���ȡ����Ŀ
		TCP_SERVICE_API TCP_HANDLE_VEC get_links_handle(TCP_HANDLE server_handle);
		//�������Ӿ�� ��ȡ�����ķ�����
		TCP_SERVICE_API TCP_HANDLE get_server_handle(TCP_HANDLE link_handle);

		//�ر�һ������ ������
		TCP_SERVICE_API void close_link(TCP_HANDLE handle);
		TCP_SERVICE_API void close_server(TCP_HANDLE handle);

		//���ò���
		TCP_SERVICE_API int set_recv_buff(size_t value);
		TCP_SERVICE_API int set_log_path(const std::string &path);
		TCP_SERVICE_API int set_thread_num(size_t value);
		TCP_SERVICE_API int set_log_lv(int lv);
	
	}
}

#endif // !TCP_SERVICE_H


