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
#include "message_bus.hpp"
#include "package_loader.h"
namespace package
{
    class PackageSystem:public System
    {
    public:
        PackageSystem();
        //系统调用
        virtual ~PackageSystem();

        //根据变量名称获取变量引用 不存在返回nullptr
        virtual std::shared_ptr<value> ref(const std::string& name)override;

        //构建变量 如果存在则覆盖
        virtual bool struct_ref(const std::string& name, std::shared_ptr<value> obj) override;

        virtual bool delete_ref(const std::string& name)override;

        //打印日志
        virtual void print_log(LogLevel level, const std::string& msg) override;

        //运行一个实例 失败返回空
        virtual bool run_instance(\
            const std::string packename, std::string instance_name="") override;

        //停止一个实例的运行
        virtual bool stop_instance(const std::string& instance_name)override;

        //扫描对应目录下的插件信息并添加
        bool scan_package(const std::string& path)override;

        //查看包名对应包是否已经加载
        virtual bool is_loaded(const std::string& name)override;

        //卸载所有的包
        bool clear_all();

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

        //消息回调
        typedef msg::Message<std::string> MessageType;
        bool on_message(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> request, \
            std::vector <std::shared_ptr<MessageType>> responses);
    private:

        //环境变量
        std::mutex ctx_lock_;
        std::unordered_map<std::string, std::shared_ptr<value>> ctx_;

        //插件名称 -> 插件加载器
        std::mutex package_loader_list_lock_;
        std::map<std::string, std::shared_ptr<PackageLoader> > package_loader_list_;

        //插件实例 实例名->实例
        std::mutex package_object_list_lock_;
        std::map<std::string ,std::shared_ptr<PackageObject> >package_object_list_;

        // 通过 System 继承
        virtual bool set_message_callback(std::shared_ptr<PackageInstance> self, std::function<void(const std::string & from, const std::string & message)>) override;
        virtual bool send_message(const std::string& to, const std::string& message) override;
    };
}
#endif