/*
    �򵥵�ָ�������
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
        //��ת
        None,
        //��
        Add,
        //���� 
        Call,
        //��ת
        JUMP,
    };
    //ָ��
    struct Cmd
    {
        struct Params
        {
            enum class ParamsType
            {
                //ֱ����
                DATA,
                //���ò���
                REF,
            };
            ParamsType type;
            std::string param;
        };
        //ָ��ֵ
        CMD_TYPE type;
        //���� ����
        std::vector<Params> params;
    };
    
    //ֵ
    typedef double number;
    typedef std::string str;
    typedef std::function<bool(std::shared_ptr<ShellNode> now)> func;
    typedef boost::variant<number, str, func> value;

    //ϵͳ����
    struct SystemCall
    {
        //���ر���
       typedef std::function<bool(std::shared_ptr<value> obj)> ReturnCall;
       ReturnCall __return__;
       //��־���
       typedef std::function<bool(const std::string& name,int level,...)> LogCall;
       LogCall __log__;
    };

    //�ű����ýڵ�
    class ShellNode
    {
        
    public:
        ShellNode();
        ~ShellNode();

        //ִ��һ��ָ��
        bool exec(Cmd cmd);

        //���ݱ������ƻ�ȡ�������� �����ڷ���nullptr
        std::shared_ptr<value> ref(const std::string& name);

        //�������� ��������򸲸�
        bool struct_ref(const std::string& name, std::shared_ptr<value> obj);
    private:
        //����
        std::hash_map<std::string, std::shared_ptr<value>> ctx_;

        //���ö�ջ
        static int call_stack;
    };
}
#endif