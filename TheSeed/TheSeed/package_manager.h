/*
    包管理器
*/
#ifndef PACKAGE_MANAGER_H_
#define PACKAGE_MANAGER_H_
#include <string>
#include <vector>
namespace package
{
    class Package
    {

    };

    //服务器节点
    typedef std::pair<std::string, int> ServerNode;
    typedef std::vector<ServerNode>  ServerNodeList;
    struct EXEC_PARAM
    {

    };
    class PackageManager
    {
    public:
        PackageManager();
        ~PackageManager();
    public:
        //执行一个操作
        int exec();
    private:
        int loader();

    };
}
#endif