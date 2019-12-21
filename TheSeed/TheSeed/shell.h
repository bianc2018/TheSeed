/*
    简单的指令解析器
    hql 2019/12/12
*/
#ifndef SHELL_H_
#define SHELL_H_

#include <boost/variant.hpp>
#include <hash_map>
namespace shell
{
    enum class CMD_TYPE
    {
        //空转
        None,
        //加
        Add,
        //调用 
        Call,
        //跳转
        JUMP,
    };
    //指令
    struct Cmd
    {
        struct Params
        {
            enum class ParamsType
            {
                //直接数
                DATA,
                //引用参数
                REF,
            };
            ParamsType type;
            std::string param;
        };
        //指令值
        CMD_TYPE type;
        //参数 名称
        std::vector<Params> params;
    };
    
    //值
    typedef double number;
    typedef std::string str;
    typedef std::function<bool(std::shared_ptr<ShellNode> now)> func;
    typedef boost::variant<number, str, func> value;

    //系统调用
    struct SystemCall
    {
        //返回变量
       typedef std::function<bool(std::shared_ptr<value> obj)> ReturnCall;
       ReturnCall __return__;
       //日志输出
       typedef std::function<bool(const std::string& name,int level,...)> LogCall;
       LogCall __log__;
    };

    //脚本调用节点
    class ShellNode
    {
        
    public:
        ShellNode();
        ~ShellNode();

        //执行一条指令
        bool exec(Cmd cmd);

        //根据变量名称获取变量引用 不存在返回nullptr
        std::shared_ptr<value> ref(const std::string& name);

        //构建变量 如果存在则覆盖
        bool struct_ref(const std::string& name, std::shared_ptr<value> obj);
    private:
        //环境
        std::hash_map<std::string, std::shared_ptr<value>> ctx_;

        //调用堆栈
        static int call_stack;
    };
}
#endif