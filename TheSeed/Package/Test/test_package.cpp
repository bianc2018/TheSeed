#include "test_package.h"
#include <thread>
TestPackage::TestPackage(std::shared_ptr<System> sys,\
    const std::string &name):PackageInstance(sys, name)
{
    sys->print_log(LogLevel::LINFO, "创建一个TestPackage实例 "+get_name());

}

TestPackage::~TestPackage()
{
    sys_call_->print_log(LogLevel::LINFO, "卸载一个TestPackage实例 "+ get_name());
}

bool TestPackage::start()
{
    sys_call_->set_event_callback(shared_from_this(),\
        std::bind(&TestPackage::event_cb, this, std::placeholders::_1));
    sys_call_->print_log(LogLevel::LINFO, "运行一个TestPackage实例 "+ get_name());
    set_flag(PACKAGE_INSTANCE_RUN_FLAG::RUNNING);
    int i = 0;
    while (package::PACKAGE_INSTANCE_RUN_FLAG::RUNNING == get_flag())
    {
        auto event = std::make_shared<SystemEvent>();
        event->type = SystemEventType::message;
        event->message = get_name();
        event->dst.instance = "test3";
        sys_call_->post_event(shared_from_this(),event);
        //std::this_thread::sleep_for(std::chrono::seconds(5));
        //break;
        ++i;
    }
    return true;
}

bool TestPackage::stop()
{
    sys_call_->print_log(LogLevel::LINFO, "停止一个TestPackage实例 "+ get_name());
    set_flag(PACKAGE_INSTANCE_RUN_FLAG::QUIT);
    return true;
}

Basic_info TestPackage::get_basic_info()
{
    Basic_info basic_info;
    basic_info.author = "Hql 2019/12/14";
    basic_info.name = "Test";
    basic_info.version = 1;
    return basic_info;
}

std::map<std::string, Ver> TestPackage::get_depends()
{
    //return get_g_info().depends;
    return std::map<std::string, Ver>();
}
void TestPackage::event_cb(std::shared_ptr<SystemEvent> message)
{
    char log[1024] = { 0 };
    snprintf(log, 1024, \
        "instance %s recv type %d messsage  %s from %s",
        get_name().c_str(), message->type, \
        message->message.c_str(), message->src.instance.c_str());
    sys_call_->print_log(LogLevel::LERR,\
        log);
}
//初始化
static bool init(std::shared_ptr<System> sys)
{
    sys->print_log(LogLevel::LINFO, "初始化 TestPackage");
    return true;
}

//产生一个实例 失败返回空
static std::shared_ptr<PackageInstance>generate(std::shared_ptr<System> sys,\
    const std::string&name)
{
    sys->print_log(LogLevel::LINFO, "实例化 TestPackage");
    return  std::dynamic_pointer_cast<PackageInstance>\
        (std::make_shared<TestPackage>(sys,name));
}

//反初始化
static bool uninit(std::shared_ptr<System> sys)
{
    sys->print_log(LogLevel::LINFO, "反初始化 TestPackage");
    return true;
}

PACKAGE_EXPORT_SYMBOLS bool Package_loader(std::shared_ptr<System> sys,\
    std::shared_ptr<Package_info> info)
{
   
    info->generate = generate;
    info->init = init;
    info->uninit = uninit;

    info->info.author = "Hql 2019/12/14";
    info->info.name = "Test";
    info->info.version = 1;

    sys->print_log(LogLevel::LINFO, \
        "Package_loader TestPackage name " + info->info.name \
        + " author " + info->info.author + " ver " + std::to_string(info->info.version));
    return true;
}
