/*
    ��������
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

    //�������ڵ�
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
        //ִ��һ������
        int exec();
    private:
        int loader();

    };
}
#endif