#include "package_loader.h"

#include "logging.hpp"
#include "util.hpp"

using namespace package;
package::PackageLoader::PackageLoader(std::shared_ptr<System> sys):\
dll_handle_(INVAILD_HANDLE), package_info_(nullptr),system_(sys)
{

}

package::PackageLoader::~PackageLoader()
{
    release();
}

bool package::PackageLoader::load(const std::string& dll_path)
{
    if (!system_)
    {
        LOG_ERR << "system_ is nullptr";
        return false;
    }

    //先释放
    release();

    auto handle = LoadPackage(dll_path.c_str());
    if (handle)
    {
        auto func = (Package_loader_fn)GetPackageApi(handle);
        if (nullptr == func)
        {
            LOG_ERR << "dll not find func "<< PACKAGE_INIT<<" path="<<dll_path;
            FreePackage(handle);
            return false;
        }

        auto package_info = std::make_shared<Package_info>();

        if (func(system_, package_info))
        {
            //加载子项
            for (auto& dep : package_info->info.depends)
            {
                if (system_->is_loaded(dep.first))
                {
                    LOG_ERR << "package is cannot load ,depends:"\
                        << dep.first << " not fond";
                    package_info.reset();
                    FreePackage(handle);
                    return false;
                }
            }
            if (!package_info|| "" == package_info->info.name)
            {
                LOG_ERR << "package is cannot load ,info is null";
                package_info.reset();
                FreePackage(handle);
                return false;
            }
            dll_handle_ = handle;
            package_info_ = package_info;
            return package_info_->init(system_);
        }
        else
        {
            LOG_ERR << "LoadPackage init error dll_path=" << dll_path;
            FreePackage(handle);
            return false;
        }
    }
    else
    {
        LOG_ERR << "LoadPackage error dll_path=" << dll_path;
        return false;
    }
    return false;
}

bool package::PackageLoader::get_package_info(const std::string& dll_path, Basic_info& info)
{
    auto handle = LoadPackage(dll_path.c_str());
    if (handle)
    {
        auto func = (Package_loader_fn)GetPackageApi(handle);
        if (nullptr == func)
        {
            LOG_ERR << "dll not find func " << PACKAGE_INIT << " path=" << dll_path;
            FreePackage(handle);
            return false;
        }

        auto package_info = std::make_shared<Package_info>();

        if (func(nullptr, package_info))
        {
            info = package_info->info;
            package_info.reset();
            FreePackage(handle);
            return true;
        }
        package_info.reset();
    }

    FreePackage(handle);
    return true;
}

std::string package::PackageLoader::get_package_name()
{
    if(!package_info_)
        return std::string();
    return package_info_->info.name;
}

std::shared_ptr<PackageObject> package::PackageLoader::generate_instance(const std::string& instance_name)
{
    if (!system_)
    {
        LOG_ERR << "system_ is nullptr";
        return nullptr;
    }

    if (!package_info_)
    {
        LOG_ERR << "package_info_ is nullptr";
        return nullptr;
    }
    auto shared_obj = std::make_shared<PackageObject>();
    shared_obj->inst = package_info_->generate(system_,instance_name);
    shared_obj->loader_ref = shared_from_this();
    if (shared_obj->init(instance_name))
        return shared_obj;

    return nullptr;
}

bool package::PackageLoader::release()
{
    if (package_info_)
    {
        package_info_->uninit(system_);
        package_info_.reset();
    }

    if (dll_handle_)
    {
        FreePackage(dll_handle_);
        dll_handle_ = INVAILD_HANDLE;
       
    }
    
    return true;
}

package::PackageObject::~PackageObject()
{
    //先stop
    if (inst)
    {
        inst->stop();
    }
    //等待运行线程关闭
    if (run_thread.joinable())
    {
        run_thread.join();
    }
    //
    msg_call_back = nullptr;
    //去除实例
    inst.reset();
    //去除加载器引用
    loader_ref.reset();
}

bool package::PackageObject::init(const std::string& instance_name)
{
    if (nullptr == inst || nullptr == loader_ref || instance_name.empty())
    {
        LOG_ERR << "init error inst or loader_ref or instance_name is empty";
        return false;
    }

    std::thread tmp([instance_name, this]()
        {
            try
            {
                LOG_DBG << "instance is running " << instance_name;
                if (false == inst->start())
                {
                    LOG_ERR << "NS_EVENT_ASYNC_VOID run a package instance error "\
                        << ",packename=" << loader_ref->get_package_name()\
                        << ",instance_name=" << instance_name;
                }

            }
            catch (const std::exception & e)
            {
                LOG_ERR << "NS_EVENT_ASYNC_VOID run a package instance exception "\
                    << "what=" << e.what() << ",packename=" << loader_ref->get_package_name()\
                    << ",instance_name=" << instance_name;
                inst->set_flag(PACKAGE_INSTANCE_RUN_FLAG::SYSTEM_ERROR);
                //obj->inst->stop();
            }
        });
    run_thread.swap(tmp);
    LOG_DBG << "run instance ok name=" << instance_name;
    return true;
}
