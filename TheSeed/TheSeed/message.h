/*
    传递数据结构
*/
#ifndef MESSAGE_H_
#define MESSAGE_H_
#include <string>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include "ns_cache.hpp"

namespace msg
{
    /*
        传输协议：
        头字段
            Method 
                方法
                必须 选项 Response 回复报文 request 请求报文
            Cmd
                指令
                请求时，自定义的报文信息
                body 可以为参数

            Token
                请求表示 
                每个请求 有独立的token ，没有token字段表示不需要服务端回复，每个回复必须携带token字段，标识其回复的请求

            BodyLen
                报文体长度
                标识 每一个报文报文内容的长度，BodyLen=0 标识为空

            Close：
                关闭标识 
                true 标识 会话已放弃 false 会话继续 默认 为true
    */

    //常用字段 

    //方法
    const std::string METHOD = "Method";
    //支持的方法 回复 token 的请求
    const std::string METHOD_Response = "Response";
    //从 目标节点 拉取数据
    const std::string METHOD_Request = "Request";

    //指令
    const std::string CMD = "Cmd";
    
    //源节点ip 端口 默认 5555
    const std::string SRC_IP = "SrcIp";
    const std::string SRC_PORT = "SrcPort";
    const std::string DST_IP = "DstIp";
    const std::string DST_PORT = "DstPort";
    //标识
    const std::string TOKEN = "Token";
    //body 长度
    const std::string BODY_LEN = "BodyLen";

    //保活 true or false
    const std::string Close = "Close";

    const std::string CRLF("\r\n");
    const std::string CRLFCRLF("\r\n\r\n");
    const std::string SPACE(" ");
    const std::string HEAD_SPLIT(": ");

    class Message
    {
    public:
        //获取body长度
        size_t get_body_len();

        //获取内容
        util::BufferCache& get_body();

        //获取值
        std::string get_head_value(const std::string& key, const std::string& notfond = "");

        //获取值
        template<typename T>
        T get_head_value(const std::string& key, const T& notfond)
        {
            std::string v = get_head_value(key);
            if (v == "")
                return notfond;
            try
            {
                auto p = boost::lexical_cast<T>(v);
                return  p;
            }
            catch (std::exception & e)
            {
                LOG_ERR << "lexical_cast error,what=" << e.what();
                return notfond;
            }
        }
        //如果值存在 则不覆盖
        bool set_head_value_not_over(const std::string& key, const std::string& value);
        
        //覆盖
        bool set_head_value(const std::string& key, const std::string& value);

        std::string get_head();

    public:
        //辅助函数

        //方法
        bool set_method(const std::string& method);
        std::string get_method(const std::string& not_find="");

        //指令
        bool set_cmd(const std::string& cmd);
        std::string get_cmd(const std::string& not_find = "");

        //指令
        bool set_token(const std::string& token);
        std::string get_token(const std::string& not_find = "");

        //设置源节点
        bool set_src_node(const std::string& ip,int port);
        std::pair<std::string, int> get_src_node(\
            const std::pair<std::string, int> &not_find = { "",-1 });

        //设置目的节点
        bool set_dst_node(const std::string& ip, int port);
        std::pair<std::string, int> get_dst_node(\
            const std::pair<std::string, int>& not_find = { "",-1 });
        
        //关闭标记
        bool set_close(bool flag);
        bool get_close(const bool& not_find = true);

        //设置文件为body内容
        bool set_body_from_file(const std::string& file_path);

        //把body内容另存为文件
        bool save_body_as_file(const std::string& file_path);

        //写body
        bool write_body(const char* data, size_t len);

        //读body 返回读取的字符数目
        std::int64_t read_body(char* buff, size_t len);
    private:
        //key - value
        //结构化的报文头 key:value
        std::unordered_map<std::string, std::string> header_;

        //报文内容
        util::BufferCache body_;
    };
}
#endif
