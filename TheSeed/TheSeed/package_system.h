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
#include "message_bus.hpp"
#include "package_loader.h"
namespace package
{
    class PackageSystem:public System
    {
    public:
        PackageSystem();
        //ϵͳ����
        virtual ~PackageSystem();

        //���ݱ������ƻ�ȡ�������� �����ڷ���nullptr
        virtual std::shared_ptr<value> ref(const std::string& name)override;

        //�������� ��������򸲸�
        virtual bool struct_ref(const std::string& name, std::shared_ptr<value> obj) override;

        virtual bool delete_ref(const std::string& name)override;

        //��ӡ��־
        virtual void print_log(LogLevel level, const std::string& msg) override;

        //����һ��ʵ�� ʧ�ܷ��ؿ�
        virtual bool run_instance(\
            const std::string packename, std::string instance_name="") override;

        //ֹͣһ��ʵ��������
        virtual bool stop_instance(const std::string& instance_name)override;

        //ɨ���ӦĿ¼�µĲ����Ϣ�����
        bool scan_package(const std::string& path)override;

        //�鿴������Ӧ���Ƿ��Ѿ�����
        virtual bool is_loaded(const std::string& name)override;

        //ж�����еİ�
        bool clear_all();

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

        //��Ϣ�ص�
        typedef msg::Message<std::string> MessageType;
        bool on_message(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> request, \
            std::vector <std::shared_ptr<MessageType>> responses);
    private:

        //��������
        std::mutex ctx_lock_;
        std::unordered_map<std::string, std::shared_ptr<value>> ctx_;

        //������� -> ���������
        std::mutex package_loader_list_lock_;
        std::map<std::string, std::shared_ptr<PackageLoader> > package_loader_list_;

        //���ʵ�� ʵ����->ʵ��
        std::mutex package_object_list_lock_;
        std::map<std::string ,std::shared_ptr<PackageObject> >package_object_list_;

        // ͨ�� System �̳�
        virtual bool set_message_callback(std::shared_ptr<PackageInstance> self, std::function<void(const std::string & from, const std::string & message)>) override;
        virtual bool send_message(const std::string& to, const std::string& message) override;
    };
}
#endif