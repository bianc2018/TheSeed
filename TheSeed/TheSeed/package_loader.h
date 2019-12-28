/*
    包加载器
    hql 2019/12/17
*/
#ifndef PACKAGE_LOADER_H_
#define PACKAGE_LOADER_H_
#include "system_define.h"
#ifdef _WIN32
#include <windows.h>
#include <thread>
//包句柄
typedef HMODULE PackageHandle;
#define INVAILD_HANDLE 0
//加载模块 string
#define LoadPackage(name) LoadLibraryA(name)
//卸载模块
#define FreePackage(handle) handle!=0?FreeLibrary(handle):1;
//获取接口
#define GetPackageApi(handle) GetProcAddress(handle,PACKAGE_INIT)
//包文件后缀
#define PackageExt ".package"
#endif
namespace package
{
    class PackageLoader;

    //目标包
    struct PackageObject
    {
        //目标名称
        std::string name;
        //实例指针
        std::shared_ptr<PackageInstance> inst;
        //对应的加载线程
        std::thread run_thread;
        //消息回调
        std::function<void(std::shared_ptr<SystemEvent> message)> msg_call_back;
        //加载器 引用
        std::shared_ptr<PackageLoader> loader_ref;

        ~PackageObject();
    private:
        //只有在加载器里才能初始化
        friend class PackageLoader;
        bool init(const std::string& in_name);
        
    };

    //插件加载信息
    class PackageLoader :public std::enable_shared_from_this<PackageLoader>
    {
    public:
        PackageLoader(std::shared_ptr<System> sys);
        ~PackageLoader();

        //
        bool load(const std::string& dll_path);

        //获取参数 
        static bool get_package_info(const std::string& dll_path, Basic_info &info);

        //获取参数 
        std::string get_package_name();

        //创建一个实例 输入实例名称
        std::shared_ptr<PackageObject> generate_instance(const std::string & instance_name);
    private:
        //释放
        bool release();
    private:
        //加载的插件句柄
        std::shared_ptr<Package_info> package_info_;
        //插件句柄
        PackageHandle dll_handle_;
        //系统指针
        std::shared_ptr<System> system_;
    };
}
#endif // PACKAGE_LOADER_H_
