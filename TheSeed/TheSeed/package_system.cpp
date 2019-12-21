#include "package_system.h"

#include "logging.hpp"
#include "util.hpp"
#include "message_bus.hpp"

using namespace package;

package::PackageSystem::PackageSystem()
{
    auto& bus = msg::MessageBus<MessageType>::instance();
    bus.set_on_request(std::bind(&PackageSystem::on_message,this,\
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    bus.start();
}

package::PackageSystem::~PackageSystem()
{
    LOG_DBG << "~PackageSystem :=" << (unsigned int)(this);
    //clear_all();
}

std::shared_ptr<value> package::PackageSystem::ref(const std::string& name)
{
    std::lock_guard<std::mutex> lk(ctx_lock_);
    auto p = ctx_.find(name);
    if(ctx_.end() == p)
        return nullptr;
    return p->second;
}

bool package::PackageSystem::struct_ref(const std::string& name, std::shared_ptr<value> obj)
{
    std::lock_guard<std::mutex> lk(ctx_lock_);
    ctx_[name] = obj;
    return true;
}

bool package::PackageSystem::delete_ref(const std::string& name)
{
    std::lock_guard<std::mutex> lk(ctx_lock_);
    ctx_.erase(name);
    return true;
}

//4k
#define LOG_BUFFER_SIZE 1024*4
void package::PackageSystem::print_log(LogLevel level, const std::string& msg)
{
    try
    {
        //va_list argptr;
        //const char* const pchar = fmt.c_str();
        //va_start(argptr, pchar);             // 初始化argptr
        //char buff[LOG_BUFFER_SIZE] = { 0 };
        //vsprintf(buff, fmt.c_str(), argptr);
        //va_end(argptr);

        //打印
        if (LogLevel::LERR == level)
            LOG_ERR << msg;
        else if (LogLevel::LWARN == level)
            LOG_WARN << msg;
        else if (LogLevel::LINFO == level)
            LOG_INFO << msg;
        else if (LogLevel::LDBG == level)
            LOG_DBG << msg;
    }
    catch (const std::exception&e)
    {
        LOG_ERR << "print_log is exception what=" << e.what();
    }
}

bool package::PackageSystem::run_instance(\
    const std::string packename, std::string instance_name)
{
    if (instance_name != "")
    {
        //已存在 返回
        auto p = get_object(instance_name);
        if (p)
            return true;
    }
    else
    {
        // 生成实例名称
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        instance_name = boost::uuids::to_string(a_uuid);

    }

    //不存在创建实例
    auto loader = get_loader(packename);
    if (loader)
    {
        //产生一个实例
        auto obj = loader->generate_instance(instance_name);
        if (!obj)
        {
            LOG_DBG << "generate instance error name=" << instance_name;
            return false;
        }
        {
            std::lock_guard<std::mutex> lk(package_object_list_lock_);
            package_object_list_[instance_name] = obj;
        }
   
        return true;
    }
    else
    {
        LOG_ERR << "not find packagename=" << packename;
        return false;
    }
}

bool package::PackageSystem::stop_instance(const std::string& instance_name)
{
    //已存在 返回对应的实例
    auto p = get_object(instance_name);
    if (!p)
    {
        return false;
    }
    //删除
    std::lock_guard<std::mutex> lk(package_object_list_lock_);
    package_object_list_.erase(instance_name);
    return true;
}

bool package::PackageSystem::scan_package(const std::string& path)
{
    LOG_INFO << "scan_package:" << path;
    //判断是不是目录 遍历目录 然后逐一加载 是否有对应的函数
    if (boost::filesystem::is_directory(path))
    {
        boost::filesystem::directory_iterator endIter;
        for (boost::filesystem::directory_iterator iter(path); iter != endIter; ++iter)
        {
            scan_package(iter->path().string());
        }
        return true;
    }
    else
    {
        if (PackageExt == boost::filesystem::extension(path))
        {
            //try load
            std::shared_ptr<PackageLoader> loader = \
                std::make_shared<PackageLoader>(get_basic_system());
            if (loader->load(path))
            {
                //加载完成
                std::lock_guard<std::mutex> lk(package_loader_list_lock_);
                package_loader_list_[loader->get_package_name()] = loader;
                return true;
            }
            else
            {
                LOG_ERR << "load dll error path=" << path;
                return false;
            }
        }
    }
    return false;
}

bool package::PackageSystem::is_loaded(const std::string& name)
{
    return nullptr != get_loader(name);
}

std::shared_ptr<System> package::PackageSystem::get_basic_system()
{
    return std::dynamic_pointer_cast<System>( get_system());
}

std::shared_ptr<PackageSystem> package::PackageSystem::get_system()
{
    static std::shared_ptr<PackageSystem> thiz = std::make_shared<PackageSystem>();
    return thiz;
}

bool package::PackageSystem::release_loader(const std::string& name)
{
    {
        //防止 在清除期间 有其他用户生成实例
        std::lock_guard<std::mutex> lk(package_loader_list_lock_);
        //删除
        package_loader_list_.erase(name);
    }
    //清空实例
    clear_instance(name);
    return true;
}

std::shared_ptr<PackageLoader> package::PackageSystem::get_loader(const std::string& name)
{
    {
        std::lock_guard<std::mutex> lk(package_loader_list_lock_);
        auto p = package_loader_list_.find(name);
        if (package_loader_list_.end() != p)
            return p->second;
    }
    return nullptr;
    
}
std::shared_ptr<PackageObject> package::PackageSystem::get_object(const std::string& name)
{
    std::lock_guard<std::mutex> lk(package_object_list_lock_);
    auto p = package_object_list_.find(name);
    if (package_object_list_.end() == p)
        return nullptr;
    return p->second;
}
//package_name 是空的释放所有的obj
bool package::PackageSystem::clear_instance(const std::string& package_name)
{
    std::lock_guard<std::mutex> lk(package_object_list_lock_);
    for (auto p = package_object_list_.begin(); p != package_object_list_.end();)
    {
        LOG_DBG << "clear " << p->first;

        auto basicinfo = p->second->inst->get_basic_info();
        
        if (package_name == basicinfo.name || package_name.empty())
        {
            p = package_object_list_.erase(p);
        }
        else
        {
            ++p;
        }
    }
    return true;
}

bool package::PackageSystem::on_message(std::shared_ptr<net::tcp::TcpLink> link, std::shared_ptr<MessageType> request, std::vector<std::shared_ptr<MessageType>> responses)
{
    return false;
}

bool package::PackageSystem::set_message_callback(std::shared_ptr<PackageInstance> self,\
    std::function<void(const std::string & from, const std::string & message)>)
{
    auto p = get_object(self->get_basic_info().name);
    return false;
}

bool package::PackageSystem::send_message(const std::string& to, const std::string& message)
{
    return false;
}

bool package::PackageSystem::clear_all()
{
    LOG_DBG << "clear_all";
    {
        std::lock_guard<std::mutex> lk(package_loader_list_lock_);
        for (auto p = package_loader_list_.begin(); p != package_loader_list_.end();)
        {
            p = package_loader_list_.erase(p);

        }
    }
    clear_instance("");
    return true;
}

