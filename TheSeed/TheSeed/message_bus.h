/*
   ���ݴ���ģ�� ���� TheSeed ֱ�ӵ����ݽ��� ���Ƿ����Ҳ�ǿͻ���
   hql 20191106
*/
#ifndef DATA_TRANSMISSION_H_
#define DATA_TRANSMISSION_H_
#include "message_session.h"
#include "tcp_server.h"
namespace msg
{
    //Ĭ�Ϸ���˿�
    const int default_port = 5555;

    typedef std::function<void(std::shared_ptr<Message> request,int err)> Handler;

    class MessageBus:public std::enable_shared_from_this<MessageBus>
    {
        //
        MessageBus();
    public:
        ~MessageBus();

        static MessageBus& instance();

        bool start(int port= default_port,int thread_num=10);

        bool set_cmd_cb(const std::string& cmd, Handler handler);

        bool send_message(std::shared_ptr<Message> message, Handler handler);
    private:

        void accept_cb(net::tcp::TcpLink::PTcpLink link, int err);

        bool set_response_handler(const std::string& token, Handler handler);

        bool remove_response_handler(const std::string& token);

        void recv_message(std::shared_ptr<MessageSession> session,std::shared_ptr<Message> message, int error_code);

        void call_cmd_handler(std::shared_ptr<Message> request, int err);

        void call_response_handler(std::shared_ptr<Message> request, int err);
    private:
        //cmd ָ����
        std::map<std::string, Handler > cmd_handlers_;

        //����˿�
        int port_;

        //token -> handler �ظ����
        std::map<std::string, Handler > response_handlers_;

        //tcp ������
        net::tcp::TcpServer server_;

        //����ָʾ��
        std::atomic_bool run_flag_;

        std::vector<std::thread> run_thread_vec_;
    };
}
#endif
