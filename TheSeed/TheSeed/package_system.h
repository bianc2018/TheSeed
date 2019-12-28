/*
    ���ϵͳ ������ز������ά������ļ������
    hql 2019/12/14
*/
#ifndef PACKAGE_SYSTEM_H_
#define PACKAGE_SYSTEM_H_
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

#include <boost/any.hpp>
#include "thread_pool.hpp"
#include "message_bus.hpp"
#include "package_loader.h"

namespace package
{
    //����͸�����ݣ����ʵ��͸������ ���ڿ��Կ�����map����
    const std::string SYSTEM_EVENT = "/SystemEvent";

    class PackageSystem :public System
    {
    public:
        PackageSystem();
        //ϵͳ����
        virtual ~PackageSystem();

        //���ϵͳ������������
        virtual value get_env(const std::string& name, value notfond)override;
        virtual bool export_env(const std::string& name, value obj) override;
        virtual bool delete_env(const std::string& name) override;

        //��ӡ��־
        virtual void print_log(LogLevel level, const std::string& msg) override;

        //����һ��ʵ�� ʧ�ܷ��ؿ�
        virtual bool run_instance(\
            const std::string packename, std::string instance_name = "") override;

        //ֹͣһ��ʵ��������
        virtual bool stop_instance(const std::string& instance_name)override;

        //ɨ���ӦĿ¼�µĲ����Ϣ�����
        bool scan_package(const std::string& path)override;

        //�鿴������Ӧ���Ƿ��Ѿ�����
        virtual bool is_loaded(const std::string& name)override;

        //ж�����еİ�
        bool clear_all();

        //������Ϣ�ص�
        virtual bool set_event_callback(\
            std::shared_ptr<PackageInstance> self, \
            std::function<void(std::shared_ptr<SystemEvent> message)> func)override;

        //��������
        virtual bool post_event(std::shared_ptr<PackageInstance> self, \
            std::shared_ptr<SystemEvent> message) override;
    public:

        //��ȡ����ʵ��
        static std::shared_ptr<System> get_basic_system();

        //��ȡϵͳʵ��
        static std::shared_ptr<PackageSystem> get_system();

    private:
        //��ȡ�����Ϣ
        std::shared_ptr<PackageLoader>  get_loader(const std::string& name);

        //��ȡ��������
        std::shared_ptr<PackageObject> get_object(const std::string& name);

        //�ͷŲ��
        bool release_loader(const std::string& name);

        //������
        //�����Ӧ������ʵ��
        bool clear_instance(const std::string& package_name);

        //*****************��Ϣͨ��*****************//
        //��Ϣ�ص�
        typedef msg::Message<std::string,\
            std::shared_ptr<package::SystemEvent>> MessageType;

        bool on_message(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> request, \
            std::vector <std::shared_ptr<MessageType>> responses);

        /********************************�¼�ϵͳ**********************************/
        //�¼��ַ���
        void event_dispatcher(std::shared_ptr<SystemEvent> event);
        
        //ע����Ϣ������
        void set_event_handler(SystemEventType type,\
            std::function<void(std::shared_ptr<SystemEvent> event)> handler_fn);

        //͸���ص�
        void event_message_hander(std::shared_ptr<SystemEvent> event);

        //�¼�Ͷ�ݵ�Ŀ�� ���ص�ֱ�ӵ��ûص����� Զ�̵ĵ�������⴫��
        bool event_send(std::shared_ptr<SystemEvent> event);

        //�¼��㲥
        bool  broadcast_event(std::shared_ptr<SystemEvent> event);

        //�����ϢĿ���ǲ��Ǳ��� ����Ƿ���true
        bool  check_is_local(std::shared_ptr<SystemEvent> event);
    private:
        //����IP�б�
        std::vector<std::string> local_ip_list_;

        //�¼�����ص�
        std::map < SystemEventType, \
            std::function<void(std::shared_ptr<SystemEvent> event)>> event_handlers_;
        //�¼��������
        util::ThreadPool<SystemEvent> event_pool_;

        //��������
        std::mutex ctx_lock_;
        std::unordered_map<std::string, value> ctx_;

        //������� -> ���������
        std::mutex package_loader_list_lock_;
        std::map<std::string, std::shared_ptr<PackageLoader> > package_loader_list_;

        //���ʵ�� ʵ����->ʵ��
        std::mutex package_object_list_lock_;
        std::map<std::string, std::shared_ptr<PackageObject> >package_object_list_;

    };
}
#endif