/*
    ���ݴ���Ự
*/
#ifndef TRANSMISSION_SESSION_H_
#define TRANSMISSION_SESSION_H_
#include "tcp_server.h"
#include "message_parser.h"
namespace msg
{
    //�Ự
    //ÿ�������� Type �ֶ��ж� ��Ϣ���� ������ token�ֶ�  �����Ƕ�Ӧĳ������

    //������
    typedef std::function<void(std::shared_ptr<Message> message, int error_code)> ReqHandler;
    typedef std::function<void(int error_code)> SendHandler;

    class MessageSession
    {
        MessageSession();
    public:
        ~MessageSession();

       static  std::shared_ptr<MessageSession> create_sesson(net::tcp::TcpLink::PTcpLink link);

        bool recv_message(ReqHandler req_handler);

        //��������
        bool send_message(std::shared_ptr<Message> message, SendHandler handler);

    private:
        bool send_head(std::shared_ptr<Message> message, SendHandler handler);

        bool send_body(std::shared_ptr<Message> message, SendHandler handler, std::shared_ptr<char> buff, int buff_len);

        //���������� ����
        void on_recv(ReqHandler req_handler,std::shared_ptr<char> recv_data, int recv_len, int err_code);
    private:
        MessageParser parser_;
        //����
        net::tcp::TcpLink::PTcpLink link_;
    };
}
#endif