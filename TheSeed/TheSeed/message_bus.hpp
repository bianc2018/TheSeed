/*
   数据传输模块 负责 TheSeed 直接的数据交互 即是服务端也是客户端
   hql 20191106
*/
#ifndef DATA_TRANSMISSION_H_
#define DATA_TRANSMISSION_H_
#include "message.hpp"
#include "message_parser.hpp"
#include "tcp_server.h"

namespace msg
{
    //默认服务端口
    const int default_port = 5555;
    extern net::tcp::io_service g_io_service;

    template<typename MessageType>
    class MessageBus:public std::enable_shared_from_this<MessageBus<MessageType>>
    {
        typedef std::function<bool(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> request, std::vector <std::shared_ptr<MessageType>> responses)> ON_REQUEST;

        typedef std::function<bool(std::shared_ptr<net::tcp::TcpLink> link, \
            std::string & request, std::string & response)> ON_ROW_REQUEST;

        //
        MessageBus():run_flag_(true), server_(g_io_service), link_num_(0), port_(default_port)
        {
            server_.set_on_accept(std::bind(&MessageBus<MessageType>::on_accept,\
                this, std::placeholders::_1));
        }
    public:
        ~MessageBus()
        {
            g_io_service.stop();
            //g_io_service.~io_context();
            run_flag_ = false;
            for (auto& t : run_thread_vec_)
            {
                if (t.joinable())
                    t.join();
            }
        }

        static MessageBus<MessageType>& instance()
        {
            // TODO: 在此处插入 return 语句
            static MessageBus<MessageType> s_instance;
            return s_instance;
        }

        //根据域名获取ip列表
        bool resolver(const std::string& host, std::vector<std::string >& ip_list)
        {
            boost::asio::ip::tcp::resolver resolver(g_io_service);
            boost::asio::ip::tcp::resolver::query query(host, "");
            boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
            boost::asio::ip::tcp::resolver::iterator end; // End marker.
            while (iter != end)
            {
                boost::asio::ip::tcp::endpoint ep = *iter;
                ip_list.push_back(ep.address().to_string());
                ++iter;
            }
            return true;
        }

        //获取本地端口
        int get_local_port()
        {
            return port_;
        }

        bool start(int port = default_port, int thread_num = 10)
        {
            port_ = port;

            if (false == server_.start(port_,
                thread_num / 2))
            {
                LOG_ERR << " tcp server start error";
                return false;
            }

            for (int i = 0; i < thread_num; ++i)
            {
                run_thread_vec_.push_back(std::thread([this]()
                    {
                        while (run_flag_)
                        {
                            g_io_service.run();
                        }
                    }
                ));
            }
            return true;
        }

        bool set_on_request(ON_REQUEST handler)
        {
            on_request_ = handler;
            return on_request_.operator bool();
        }

        bool send_message(std::shared_ptr<MessageType> message)
        {
            if (message)
            {
                auto dst = message->get_head().dst;

                //无目标
                if ("" == dst)
                    return false;

                //看一下是否有 现成的连接
                auto link = get_link(dst);
                if (nullptr == link)
                {
                    //没有则创建新连接
                    link = net::tcp::TcpLink::generate();
                    if (!link)
                        return false;

                    link->set_on_connect([this, link, message](bool successd)
                        {
                            on_connect(link, message, successd);
                        });
                    std::string ip;
                    int port = default_port;
                    if (!parse_dst(dst, ip, port))
                    {
                        return false;
                    }
                    link->connect(g_io_service, ip, port);

                }
                else
                {
                    link->async_send(message->to_string());
                }
                return true;
            }
            return false;
        }

        //回调中改变当前通道属性
        //设置裸流通道
        static bool set_row_handler(std::shared_ptr<net::tcp::TcpLink> link,
            ON_ROW_REQUEST on_raw_req, const std::string& send_msg = "")
        {
            if (link && link->is_active())
            {
                auto parser = std::make_shared<msg::MessageParser<MessageType>>();
                link->set_on_message(std::bind(on_raw_req, link, \
                    std::placeholders::_1, std::placeholders::_2));
                link->async_send(send_msg);
                return true;
            }
            return false;
        }

        //设置为指令通道
        static bool set_req_handler(std::shared_ptr<net::tcp::TcpLink> link,
            std::shared_ptr<MessageType> message = nullptr)
        {
            if (link && link->is_active())
            {
                auto parser = std::make_shared<msg::MessageParser<MessageType>>();
                link->set_on_message(std::bind(&MessageBus::on_message, &MessageBus::instance(), link, parser, \
                    std::placeholders::_1, std::placeholders::_2));
                if (message)
                {
                    auto msg = message->to_string();
                    link->async_send(msg);
                }
                return true;
            }
            return false;
        }
    private:
        //服务端接收到链接
        void on_accept(std::shared_ptr<net::tcp::TcpLink> link)
        {
            ++link_num_;
            add_link(link);
            LOG_DBG << link_num_ << " accept link " << link->get_remote_ip() << ":" << link->get_remote_port();
            link->set_on_close(std::bind(&MessageBus::on_close, this, link, std::placeholders::_1));
            set_req_handler(link);
            link->start();
        }

        //链接关闭
        void on_close(std::shared_ptr<net::tcp::TcpLink> link, bool accord)
        {
            --link_num_;
            del_link(link);
            LOG_DBG << link_num_ << " link close " << link->get_remote_ip() << ":" << link->get_remote_port();
        }

        //链接建立
        void on_connect(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> message, bool successd)
        {
            if (successd)
            {
                ++link_num_;
                add_link(link);
                LOG_DBG << link_num_ << " connect link " << link->get_remote_ip() << ":" << link->get_remote_port();
                link->set_on_close(std::bind(&MessageBus::on_close, this, link, std::placeholders::_1));

                set_req_handler(link);

                std::string msg;
                if (message)
                {
                    msg = std::move(message->to_string());
                }
                link->start(msg);
            }
            else
            {
                if (message)
                {
                    LOG_ERR << "connect error dst " << message->get_head().dst;
                }
                else
                {
                    LOG_ERR << "connect error dst is null";
                }
            }
        }


        //收到信息 数据包 带解析器
        bool on_message(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<msg::MessageParser<MessageType>> parser, \
            std::string& in, std::string& out)
        {
            auto ret = parser->parse(in.c_str(), in.size());
            if (PARSE_ERROR == ret)
            {
                LOG_ERR << "message parse error str=" << in;
                parser->reset();
            }

            //处理所有数据
            bool keepalive = true;
            while (true)
            {
                auto req = parser->get_message();
                if (req)
                {
                    std::vector<std::shared_ptr<MessageType>> responses;

                    //只要一个false断开连接
                    if (!on_request(link, req, responses))
                        keepalive = false;

                    for (auto& res : responses)
                    {
                        auto msg = res->to_string();
                        if ("" == msg)
                        {
                            //加上分隔符
                            out += msg;
                        }
                    }
                }
                else
                {
                    break;
                }
            }

            return keepalive;

           
        }

        //收到裸数据
        //bool on_raw(std::string& in, std::string& out);

        //请求处理回调
        bool on_request(std::shared_ptr<net::tcp::TcpLink> link, \
            std::shared_ptr<MessageType> request, \
            std::vector <std::shared_ptr<MessageType>> responses)
        {
            bool keepalive = false;
            if (on_request_)
                keepalive = on_request_(link, request, responses);
            return keepalive;
        }

        //连接管理 dst 127.0.0.1:80
        bool add_link(std::shared_ptr<net::tcp::TcpLink> link)
        {
            if (link && link->is_active())
            {
                auto dst = link->get_local_ip() + ":" + std::to_string(link->get_remote_port());
                auto p = get_link(dst);

                if (p)
                    p->close();

                std::lock_guard<std::mutex> lk(tcp_map_lock_);
                tcp_map_[dst] = link;
                return true;
            }
            return false;
        }

        std::shared_ptr<net::tcp::TcpLink> get_link(const std::string& dst)
        {
            std::lock_guard<std::mutex> lk(tcp_map_lock_);
            auto p = tcp_map_.find(dst);
            if (tcp_map_.end() == p)
                return nullptr;
            return p->second;
        }
        bool del_link(std::shared_ptr<net::tcp::TcpLink> link)
        {
            std::lock_guard<std::mutex> lk(tcp_map_lock_);
            auto dst = link->get_local_ip() + ":" + std::to_string(link->get_remote_port());
            auto p = tcp_map_.erase(dst);
            return true;
        }
        //dst 127.0.0.1:80
        bool parse_dst(const std::string& dst, std::string& ip, int& port)
        {

            try
            {
                port = default_port;
                auto p = dst.find(":");
                if (std::string::npos != p)
                {
                    port = std::stoi(dst.substr(p));
                }
                ip = dst.substr(0, p);
                return true;
            }
            catch (const std::exception & e)
            {
                LOG_ERR << "exception what=" << e.what();
                return false;
            }

        }
    private:

        //请求处理回调
        ON_REQUEST on_request_;

        //服务端口
        int port_;

        //tcp 服务器
        net::tcp::TcpServer server_;

        //运行指示器
        std::atomic_bool run_flag_;

        std::vector<std::thread> run_thread_vec_;

        //连接数
        std::atomic_int64_t link_num_;

        //link dst->link
        std::mutex tcp_map_lock_;
        std::map<std::string, std::shared_ptr<net::tcp::TcpLink>> tcp_map_;
    };
}
#endif
