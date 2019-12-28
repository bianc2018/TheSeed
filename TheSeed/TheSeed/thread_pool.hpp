/*
    һ�����׵��̳߳�
    hql 2019/12/24
*/
#ifndef UTIL_THREAD_POOL_HPP_
#define UTIL_THREAD_POOL_HPP_
#include <functional>
#include "safe_queue.hpp"
namespace util
{
    template<typename T>
    class ThreadPool
    {
    public:
        //�̳߳ع�������
        typedef std::function<void(std::shared_ptr<T> t)> THREAD_POOL_WORKER;

        ThreadPool():run_flag_(true), thread_num_(ThreadPool<T>::default_thread_num())
        {}
        ~ThreadPool()
        {
            quit();
        }
        size_t queue_size()
        {
            return queue_.size();
        }
        bool start(int thread_num = 0)
        {
            if (!worker_)
                return false;

            if (thread_num > 0)
                thread_num_ = thread_num;

            for (int i = 0; i < thread_num_; ++i)
            {
                run_thread_vector_.push_back(std::thread(&ThreadPool::run_func, this));
            }
            return true;
        }

        bool submit(THREAD_POOL_WORKER fn)
        {
            if (fn)
            {
                worker_ = fn;
                return true;
            }
            else
                return false;
        }

        bool push(std::shared_ptr<T> data)
        {
            return queue_.push(data);
        }

        void quit()
        {
            if (run_flag_)
            {
                run_flag_ = false;
                queue_.release();
                for (auto& t : run_thread_vector_)
                {
                    if (t.joinable())
                        t.join();
                }
            }
        }

        //Ĭ���߳���
        static size_t default_thread_num()
        {
            return std::thread::hardware_concurrency() * 2 + 2;
        }
    private:
        //���к���
        void run_func()
        {
            while (run_flag_)
            {
                auto p = queue_.pop();
                if (p)
                {
                    if (worker_)
                    {
                        worker_(p);
                    }
                }
            }
        }
    private:
        //�߳���
        int thread_num_;

        //����flag
        std::atomic_bool run_flag_;

        SafeQueue<T> queue_;

        //�����߳�
        THREAD_POOL_WORKER worker_;

        //�����߳�
        std::vector<std::thread> run_thread_vector_;
    };
}
#endif
