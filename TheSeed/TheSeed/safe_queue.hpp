/*
    通用线程安全队列
    hql 2019/12/24
*/
#ifndef UTIL_SAFE_QUEUE_HPP_
#define UTIL_SAFE_QUEUE_HPP_
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include "logging.hpp"
namespace util
{
    template<typename T,typename size_type=size_t>
    class SafeQueue
    {
        struct QueueNode
        {
            std::shared_ptr<T> data=nullptr;
            std::shared_ptr<QueueNode> next = nullptr;
        };
        //节点指针
        typedef std::shared_ptr<QueueNode> NodePtr;
        //数据指针
        typedef std::shared_ptr<T> DataPtr;

    public:
        SafeQueue():exit_flag_(false), queue_beg_(nullptr),queue_end_(nullptr), size_(0)
        {}

        size_type size()
        {
            //std::unique_lock<std::mutex> lock(data_queue_lock_);
            //return data_queue_.size();
            
            //return data_queue_.empty
            return size_;
        }

        bool push(const T& data)
        {
            auto data = std::make_shared<T>(data);
            return push(data);

        }
        
        bool push(std::shared_ptr<T> data)
        {
            if (nullptr == data||exit_flag_)
            {
                return false;
            }
            NodePtr node = std::make_shared<QueueNode>();
            node->data = data;
            std::unique_lock<std::mutex> lock(data_queue_lock_);
            //尾部加数据
            if (queue_end_)
            {
                queue_end_->next = node;
                //前移
                queue_end_ = queue_end_->next;
            }
            else
            {
                //只有一个节点
                queue_end_ = node;
                queue_beg_ = node;
            }
            ++size_;
            data_queue_condition_.notify_one();
            return true;
        }
        DataPtr pop()
        {
            //unique_lock<mutex>
            std::unique_lock<std::mutex> lock(data_queue_lock_);
            data_queue_condition_.wait(lock, \
                [this]
                {
                    //已退出
                    if (exit_flag_)
                        return true;
                    //有数据
                    if (size_!=0)
                        return true;
                    return false;
                });
            LOG_DBG << size_ << " " << (queue_beg_ == nullptr)<<" "<<(queue_end_ == nullptr);
            if (exit_flag_ || 0 == size_)
                return nullptr;
            
            //从头部取数据
            auto p = queue_beg_;
            if (queue_beg_)
            {
                queue_beg_ = queue_beg_->next;
                if (!queue_beg_)
                    queue_end_ = nullptr;
                --size_;
                return p->data;
            }
            return nullptr;
        }

        DataPtr try_pop(size_t time_out)
        {
            //unique_lock<mutex>
            std::unique_lock<std::mutex> lock(data_queue_lock_);
            auto wait_res =\
                data_queue_condition_.wait_for(lock, std::chrono::seconds(time_out),\
                [this]
                {
                    //已退出
                    if (exit_flag_)
                        return true;
                    //有数据
                    if (size_ != 0)
                        return true;
                    return false;
                });

            if (exit_flag_ || 0 == size_ || false == wait_res)
                return nullptr;

            //从头部取数据
            auto p = queue_beg_;
            if (queue_beg_)
            {
                queue_beg_ = queue_beg_->next;
                if (queue_beg_)
                    queue_end_ = nullptr;
                ++size_;
                return p->data;
            }
            return nullptr;
        }
        //释放锁
        void release()
        {
            if (!exit_flag_)
            {
                exit_flag_ = true;
                data_queue_condition_.notify_all();
                
                std::unique_lock<std::mutex> lock(data_queue_lock_);
                auto d = queue_end_.use_count();
                while (queue_beg_)
                {
                    queue_beg_ = queue_beg_->next;
                }
                //queue_beg_ = nullptr;
                queue_end_ = nullptr;
                size_ = 0;
            }
        }
        ~SafeQueue()
        {
            release();
        }
    private:
        std::atomic_bool exit_flag_;

        //任务列表
        //锁
        std::mutex data_queue_lock_;
        //条件变量 
        std::condition_variable data_queue_condition_;
        //任务队列
        //std::list<T> data_queue_;
        //头尾节点
        NodePtr queue_beg_;
        NodePtr queue_end_;
        //数目
        size_type size_;
    };
}
#endif
