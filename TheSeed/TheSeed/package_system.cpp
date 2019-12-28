#include "package_system.h"

#include "logging.hpp"
#include "util.hpp"

//序列化
namespace boost {
    namespace serialization
    {
        template<typename Archive>
        void serialize(Archive& ar, package::SystemEvent& msg, const unsigned int version)
        {
            ar& msg.type;
            ar& msg.src.ip;
            ar& msg.src.port;
            ar& msg.src.instance;
            ar& msg.dst.ip;
            ar& msg.dst.port;
            ar& msg.dst.instance;

            ar& msg.message;
        }
    }
}


#include "message_bus.hpp"

using namespace package;

package::PackageSystem::PackageSystem()
{
    auto& bus = msg::MessageBus<MessageType>::instance();
    bus.set_on_request(std::bind(&PackageSystem::on_message,this,\
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    bus.start();

    //任务池
    event_pool_.submit(std::bind(&PackageSystem::event_dispatcher, this,\
        std::placeholders::_1));
    set_event_handler(SystemEventType::message, \
        std::bind(&PackageSystem::event_message_hander, this, \
            std::placeholders::_1));
    event_pool_.start();

    bus.resolver(boost::asio::ip::host_name(), local_ip_list_);
    local_ip_list_.push_back(boost::asio::ip::address_v4::loopback().to_string());
    for (auto& p : local_ip_list_)
    {
        LOG_DBG << "local ip=" << p;
    }
}

package::PackageSystem::~PackageSystem()
{
    LOG_DBG << "~PackageSystem :=" << (unsigned int)(this);
    //clear_all();
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
    //LOG_INFO << "scan_package:" << path;
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

bool package::PackageSystem::on_message(std::shared_ptr<net::tcp::TcpLink> link,\
    std::shared_ptr<MessageType> request, std::vector<std::shared_ptr<MessageType>> responses)
{
    if (request)
    {
        LOG_INFO << "recv a request "<< request->get_head().path\
            <<" from " << request->get_head().src;
        if (SYSTEM_EVENT == request->get_head().path)
        {
            std::shared_ptr<SystemEvent> event;
            if (request->getbody(event))
            {
                return event_pool_.push(event);
            }
            else
            {
                LOG_ERR << "/SystemEvent body must be class SystemEvent ";
            }
            return true;
        }
        else
        {
            LOG_ERR << "no deal path=" << request->get_head().path;
            return true;
        }
        
    }
    return true;
}

void package::PackageSystem::event_dispatcher(std::shared_ptr<SystemEvent> event)
{
    LOG_DBG << "event queue.size=" << event_pool_.queue_size();
    auto p = event_handlers_.find(event->type);
    if (event_handlers_.end() == p)
    {
        LOG_DBG << "not fond handler type=" << (int)event->type;
        event_send(event);
    }
    else
    {
        if (p->second)
        {
            p->second(event);
        }
        else
        {
            LOG_DBG << "handler is invaildable type=" << (int)event->type;
            event_send(event);
        }
    }
}

void package::PackageSystem::set_event_handler(SystemEventType type,
    std::function<void(std::shared_ptr<SystemEvent> event)> handler_fn)
{
    event_handlers_[type] = handler_fn;
}

void package::PackageSystem::event_message_hander(std::shared_ptr<SystemEvent> event)
{
    LOG_DBG << "recv message -> " << event->message\
        << " from " << event->src.ip << ":" << event->src.port << ":" << event->src.instance\
        << " to " << event->dst.ip << ":" << event->dst.port << ":" << event->dst.instance;
    event_send(event);
}

bool package::PackageSystem::event_send(std::shared_ptr<SystemEvent> event)
{
    //目标ip为空即表示发送给本地实例
    //区分环回地址
    if (event->dst.ip.empty()|| check_is_local(event))
    {
        if (event->dst.instance.empty())
        {
            //表示广播
            LOG_DBG << "broadcast_event from "\
                << event->src.ip << ":" << event->src.port << ":" << event->src.instance;
            return broadcast_event(event);
        }
        else
        {
            auto obj = get_object(event->dst.instance);
            if (obj)
            {
                LOG_DBG << "event " << (int)event->type\
                    << " from " << event->src.ip << ":" << event->src.port << ":" << event->src.instance\
                    << " to " << event->dst.ip << ":" << event->dst.port << ":" << event->dst.instance;
                if (obj->msg_call_back)
                {
                    obj->msg_call_back(event);
                    return true;
                }
                return false;
            }
            else
            {
                LOG_ERR << "not find obj name=" << event->dst.instance;
                return false;
            }
        }
    }
    //远程
    else
    {
        
        auto& bus = msg::MessageBus<MessageType>::instance();
        auto message = std::make_shared<MessageType>();
        //message->get_head().src = event.src.ip + std::to_string(event.src.port);
        message->get_head().dst = event->dst.ip +":"+ std::to_string(event->dst.port);
        message->setbody(event);
        LOG_DBG << "send a event message to" << message->get_head().dst;
        return bus.send_message(message);
    }
    return false;
}

bool package::PackageSystem::broadcast_event(std::shared_ptr<SystemEvent> event)
{
    std::lock_guard<std::mutex> lk(package_object_list_lock_);
    for (auto p = package_object_list_.begin(); p != package_object_list_.end();)
    {
        auto obj = p->second;
        if (obj && obj->msg_call_back)
        {
            auto tmp = event;
            obj->msg_call_back(tmp);
        }
        ++p;
    }
    return true;
}

bool package::PackageSystem::check_is_local(std::shared_ptr<SystemEvent> event)
{
    if (event->dst.port == 0 || event->dst.ip.empty())
        return true;

    if (msg::MessageBus<MessageType>::instance().get_local_port() == event->dst.port)
    {
        return false;
    }

    for (auto& p : local_ip_list_)
    {
        if (event->dst.ip == p)
        {
            return true;
        }
    }
    return false;
}

bool package::PackageSystem::set_event_callback(std::shared_ptr<PackageInstance> self,\
    std::function<void(std::shared_ptr<SystemEvent> message)> func)
{
    if (self)
    {
        //找到对应对象 设置回调
        auto p = get_object(self->get_name());
        if (p)
        {
            p->msg_call_back = func;
            return true;
        }
    }
    return false;
}

//推送一个数据请求
bool package::PackageSystem::post_event(std::shared_ptr<PackageInstance> self, \
    std::shared_ptr<SystemEvent> event)
{
    event->src.instance = self->get_name();
    /*while (event_pool_.queue_size() > 100000)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));*/
    return event_pool_.push(event);
}

value package::PackageSystem::get_env(const std::string& name, value notfond)
{
    //, value notfond
    std::lock_guard<std::mutex> lk(ctx_lock_);
    auto p = ctx_.find(name);
    if (ctx_.end() == p)
        return notfond;
    return p->second;
}

bool package::PackageSystem::export_env(const std::string& name, value obj)
{
    std::lock_guard<std::mutex> lk(ctx_lock_);
    ctx_[name] = obj;
    return true;
}

bool package::PackageSystem::delete_env(const std::string& name)
{
    std::lock_guard<std::mutex> lk(ctx_lock_);
    ctx_.erase(name);
    return true;
}

bool package::PackageSystem::clear_all()
{
    LOG_DBG << "clear_all";
    event_pool_.quit();
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

