/*
    插件系统 负责加载插件，并维护插件的加载情况
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
    //数据透传数据，插件实例透传数据 后期可以考虑用map管理
    const std::string SYSTEM_EVENT = "/SystemEvent";

    class PackageSystem :public System
    {
    public:
        PackageSystem();
        //系统调用
        virtual ~PackageSystem();

        //插件系统环境变量操作
        virtual value get_env(const std::string& name, value notfond)override;
        virtual bool export_env(const std::string& name, value obj) override;
        virtual bool delete_env(const std::string& name) override;

        //打印日志
        virtual void print_log(LogLevel level, const std::string& msg) override;

        //运行一个实例 失败返回空
        virtual bool run_instance(\
            const std::string packename, std::string instance_name = "") override;

        //停止一个实例的运行
        virtual bool stop_instance(const std::string& instance_name)override;

        //扫描对应目录下的插件信息并添加
        bool scan_package(const std::string& path)override;

        //查看包名对应包是否已经加载
        virtual bool is_loaded(const std::string& name)override;

        //卸载所有的包
        bool clear_all();

        //设置消息回调
        virtual bool set_event_callback(\
            std::shared_ptr<PackageInstance> self, \
            std::function<void(std::shared_ptr<SystemEvent> message)> func)override;

        //推送数据
        virtual bool post_event(std::shared_ptr<PackageInstance> self, \
            std::shared_ptr<SystemEvent> message) override;
    public:

        //获取基础实例
        static std::shared_ptr<System> get_basic_system();

        //获取系统实例
        static std::shared_ptr<PackageSystem> get_system();

    private:
        //获取插件信息
        std::shared_ptr<PackageLoader>  get_loader(const std::string& name);

        //获取对象名称
        std::shared_ptr<PackageObject> get_object(const std::string& name);

        //释放插件
        bool release_loader(const std::string& name);

        //清理函数
        //清理对应包名的实例
        bool clear_instance(const std::string& package_name);

        //*****************消息通信*****************//
        //消息回调
        typedef msg::Message<std::string,\
            std::shared_ptr<package::SystemEvent>> MessageType;

        bool on_message(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> request, \
            std::vector <std::shared_ptr<MessageType>> responses);

        /********************************事件系统**********************************/
        //事件分发器
        void event_dispatcher(std::shared_ptr<SystemEvent> event);
        
        //注册消息处理器
        void set_event_handler(SystemEventType type,\
            std::function<void(std::shared_ptr<SystemEvent> event)> handler_fn);

        //透传回调
        void event_message_hander(std::shared_ptr<SystemEvent> event);

        //事件投递到目标 本地的直接调用回调函数 远程的调用网络库传输
        bool event_send(std::shared_ptr<SystemEvent> event);

        //事件广播
        bool  broadcast_event(std::shared_ptr<SystemEvent> event);

        //检查消息目标是不是本地 如果是返回true
        bool  check_is_local(std::shared_ptr<SystemEvent> event);
    private:
        //本地IP列表
        std::vector<std::string> local_ip_list_;

        //事件处理回调
        std::map < SystemEventType, \
            std::function<void(std::shared_ptr<SystemEvent> event)>> event_handlers_;
        //事件处理队列
        util::ThreadPool<SystemEvent> event_pool_;

        //环境变量
        std::mutex ctx_lock_;
        std::unordered_map<std::string, value> ctx_;

        //插件名称 -> 插件加载器
        std::mutex package_loader_list_lock_;
        std::map<std::string, std::shared_ptr<PackageLoader> > package_loader_list_;

        //插件实例 实例名->实例
        std::mutex package_object_list_lock_;
        std::map<std::string, std::shared_ptr<PackageObject> >package_object_list_;

    };
}
#endif