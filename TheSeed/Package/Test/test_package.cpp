#include "test_package.h"

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
    sys_call_->print_log(LogLevel::LINFO, "运行一个TestPackage实例 "+ get_name());
    return true;
}

bool TestPackage::stop()
{
    sys_call_->print_log(LogLevel::LINFO, "停止一个TestPackage实例 "+ get_name());
    return true;
}
static Basic_info get_g_info()
{
    Basic_info basic_info;
    basic_info.author = "Hql 2019/12/14";
    basic_info.name = "Test";
    basic_info.version = 1;
    return basic_info;
}
Basic_info TestPackage::get_basic_info()
{
    return get_g_info();
}

const std::map<std::string, Ver>& TestPackage::get_depends()
{
    return get_g_info().depends;
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
    info->info = get_g_info();
    sys->print_log(LogLevel::LINFO, \
        "Package_loader TestPackage name " + info->info.name \
        + " author " + info->info.author + " ver " + std::to_string(info->info.version));
    return true;
}
