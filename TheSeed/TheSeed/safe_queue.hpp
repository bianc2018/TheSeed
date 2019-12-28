/*
    ͨ���̰߳�ȫ����
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
        //�ڵ�ָ��
        typedef std::shared_ptr<QueueNode> NodePtr;
        //����ָ��
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
            //β��������
            if (queue_end_)
            {
                queue_end_->next = node;
                //ǰ��
                queue_end_ = queue_end_->next;
            }
            else
            {
                //ֻ��һ���ڵ�
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
                    //���˳�
                    if (exit_flag_)
                        return true;
                    //������
                    if (size_!=0)
                        return true;
                    return false;
                });
            LOG_DBG << size_ << " " << (queue_beg_ == nullptr)<<" "<<(queue_end_ == nullptr);
            if (exit_flag_ || 0 == size_)
                return nullptr;
            
            //��ͷ��ȡ����
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
                    //���˳�
                    if (exit_flag_)
                        return true;
                    //������
                    if (size_ != 0)
                        return true;
                    return false;
                });

            if (exit_flag_ || 0 == size_ || false == wait_res)
                return nullptr;

            //��ͷ��ȡ����
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
        //�ͷ���
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

        //�����б�
        //��
        std::mutex data_queue_lock_;
        //�������� 
        std::condition_variable data_queue_condition_;
        //�������
        //std::list<T> data_queue_;
        //ͷβ�ڵ�
        NodePtr queue_beg_;
        NodePtr queue_end_;
        //��Ŀ
        size_type size_;
    };
}
#endif
