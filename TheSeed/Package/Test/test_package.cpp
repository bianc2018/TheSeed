#include "test_package.h"

TestPackage::TestPackage(std::shared_ptr<System> sys,\
    const std::string &name):PackageInstance(sys, name)
{
    sys->print_log(LogLevel::LINFO, "����һ��TestPackageʵ�� "+get_name());
}

TestPackage::~TestPackage()
{
    sys_call_->print_log(LogLevel::LINFO, "ж��һ��TestPackageʵ�� "+ get_name());
}

bool TestPackage::start()
{
    sys_call_->print_log(LogLevel::LINFO, "����һ��TestPackageʵ�� "+ get_name());
    return true;
}

bool TestPackage::stop()
{
    sys_call_->print_log(LogLevel::LINFO, "ֹͣһ��TestPackageʵ�� "+ get_name());
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
//��ʼ��
static bool init(std::shared_ptr<System> sys)
{
    sys->print_log(LogLevel::LINFO, "��ʼ�� TestPackage");
    return true;
}

//����һ��ʵ�� ʧ�ܷ��ؿ�
static std::shared_ptr<PackageInstance>generate(std::shared_ptr<System> sys,\
    const std::string&name)
{
    sys->print_log(LogLevel::LINFO, "ʵ���� TestPackage");
    return  std::dynamic_pointer_cast<PackageInstance>\
        (std::make_shared<TestPackage>(sys,name));
}

//����ʼ��
static bool uninit(std::shared_ptr<System> sys)
{
    sys->print_log(LogLevel::LINFO, "����ʼ�� TestPackage");
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
