/*
    ��������
    hql 2019/12/17
*/
#ifndef PACKAGE_LOADER_H_
#define PACKAGE_LOADER_H_
#include "system_define.h"
#ifdef _WIN32
#include <windows.h>
#include <thread>
//�����
typedef HMODULE PackageHandle;
#define INVAILD_HANDLE 0
//����ģ�� string
#define LoadPackage(name) LoadLibraryA(name)
//ж��ģ��
#define FreePackage(handle) handle!=0?FreeLibrary(handle):1;
//��ȡ�ӿ�
#define GetPackageApi(handle) GetProcAddress(handle,PACKAGE_INIT)
//���ļ���׺
#define PackageExt ".package"
#endif
namespace package
{
    class PackageLoader;

    //Ŀ���
    struct PackageObject
    {
        //Ŀ������
        std::string name;
        //ʵ��ָ��
        std::shared_ptr<PackageInstance> inst;
        //��Ӧ�ļ����߳�
        std::thread run_thread;
        //��Ϣ�ص�
        std::function<void(std::shared_ptr<SystemEvent> message)> msg_call_back;
        //������ ����
        std::shared_ptr<PackageLoader> loader_ref;

        ~PackageObject();
    private:
        //ֻ���ڼ���������ܳ�ʼ��
        friend class PackageLoader;
        bool init(const std::string& in_name);
        
    };

    //���������Ϣ
    class PackageLoader :public std::enable_shared_from_this<PackageLoader>
    {
    public:
        PackageLoader(std::shared_ptr<System> sys);
        ~PackageLoader();

        //
        bool load(const std::string& dll_path);

        //��ȡ���� 
        static bool get_package_info(const std::string& dll_path, Basic_info &info);

        //��ȡ���� 
        std::string get_package_name();

        //����һ��ʵ�� ����ʵ������
        std::shared_ptr<PackageObject> generate_instance(const std::string & instance_name);
    private:
        //�ͷ�
        bool release();
    private:
        //���صĲ�����
        std::shared_ptr<Package_info> package_info_;
        //������
        PackageHandle dll_handle_;
        //ϵͳָ��
        std::shared_ptr<System> system_;
    };
}
#endif // PACKAGE_LOADER_H_
