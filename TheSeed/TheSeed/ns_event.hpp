/*
	事件系统

*/
#ifndef NS_EVENT_H_
#define NS_EVENT_H_

#include <thread>
#include <mutex>
#include <list>
#include <future>
#include <chrono>
#include <map>
#include <unordered_map>
#include <condition_variable>

//#include <time.h>
namespace ns
{
	namespace event
	{
		class Event
		{
			typedef std::thread run_thread_item;
			typedef  std::function<void()> task_func;
			
		public:
			//单例
			static Event& instance()
			{
				static Event e;
				return e;
				
			}
		public:
			template<typename Func, typename ...Types>
			void async_void(Func func, Types&& ... args)
			{
				post(std::bind(func, std::forward<Types>(args)...));
			}
			
            template<typename Func, typename ...Types>
            void async_void_first(Func func, Types&& ... args)
            {
                post_first(std::bind(func, std::forward<Types>(args)...));
            }

            // 添加新的任务到任务队列
            template<class F, class... Args>
            auto async(F&& f, Args&& ... args)
                -> std::future<typename std::result_of<F(Args...)>::type>
            {
                // 获取函数返回值类型        
                using return_type = typename std::result_of<F(Args...)>::type;

                // 创建一个指向任务的只能指针
                auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                std::future<return_type> res = task->get_future();
                //推送
                post([task]() { (*task)(); });
                return res;
            }

            // 添加新的任务到优先
            template<class F, class... Args>
            auto async_first(F&& f, Args&& ... args)
                -> std::future<typename std::result_of<F(Args...)>::type>
            {
                // 获取函数返回值类型        
                using return_type = typename std::result_of<F(Args...)>::type;

                // 创建一个指向任务的只能指针
                auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                std::future<return_type> res = task->get_future();
                //推送
                post_first([task]() { (*task)(); });
                return res;
            }

            size_t cpu_num() 
			{
				return std::thread::hardware_concurrency()*2+2;
			}

            ~Event()
			{
                exit_flag_ = true;
				run_flag_ = false;
                list_condition_.notify_all();
				for (auto& t : run_thread_vector_)
				{
					if (t.joinable())
						t.join();
				}
				run_thread_vector_.clear();
			}
		private:
			Event()
			{
                exit_flag_ = false;
				run_flag_ = true;
                //async_void(&Event::updata_time, this, 5)-1;
				auto cpu_num =  std::thread::hardware_concurrency()*2+2-1;
				//LOG_DBG <<"hardware_concurrency = "<<cpu_num;
				for (size_t i = 0; i < cpu_num; ++i)
				{
					run_thread_vector_.push_back(std::thread(&Event::worker, this));
				}

			}
            void start()
            {
                run_flag_ = true;
                list_condition_.notify_all();
            }
            void stop()
            {
                run_flag_ = false;
                list_condition_.notify_all();
            }
			void post(task_func func)
			{
                std::unique_lock<std::mutex> lock(list_lock_);
                task_list_.push_back(std::move(func));
                list_condition_.notify_one();
			}
            void post_first(task_func func)
            {
                std::unique_lock<std::mutex> lock(list_lock_);
                task_list_.push_front(std::move(func));
                list_condition_.notify_one();
            }
			void worker()
			{
				while (!exit_flag_)
				{
                    // task是一个函数类型，从任务队列接收任务
                    task_func task;

                    {
                       // printf("worker is lock!\n");
                        //给互斥量加锁，锁对象生命周期结束后自动解锁
                        std::unique_lock<std::mutex> lock(list_lock_);

                        //（1）当匿名函数返回false时才阻塞线程，阻塞时自动释放锁。
                        //（2）当匿名函数返回true且受到通知时解阻塞，然后加锁。
                        list_condition_.wait(lock, \
                            [this] 
                            {
                                //已退出
                                if (exit_flag_)
                                    return true;
                                //未退出 且 任务链表不为空 且在运行
                                if (!task_list_.empty()&&run_flag_)
                                    return true;
                                return false;
                            });

                        if (exit_flag_ && task_list_.empty())
                            break;

                        //从任务队列取出一个任务
                        task = std::move(task_list_.front());
                        task_list_.pop_front();
                      //  printf("worker is unlock!\n");
                        //printf("[Event] task_list_ size %d\n", task_list_.size());
                    }                           
                    // 自动解锁
                    //printf("test in \n");
                    try
                    {
                       // printf("[Event] run task in\n");
                        task();                      // 执行这个任务
                       // printf("[Event] run task out\n");
                    }
                    catch (std::exception &e)
                    {
                        printf("[Event] run task err,msg:%s\n",e.what());
                    }
                    //printf("test out \n");
				}
			
                printf("worker is exit!\n");
            }
            
        private:
			//工作线程
			std::vector<run_thread_item> run_thread_vector_;
			
            std::atomic_bool exit_flag_;
            std::atomic_bool run_flag_;

            //任务列表
            //锁
            std::mutex list_lock_;
            //条件变量 
            std::condition_variable list_condition_;             
            //任务队列
            std::list<task_func> task_list_;

		};
	}

}

#define NS_EVENT ns::event::Event::instance()
#define NS_EVENT_ASYNC_VOID(...) NS_EVENT.async_void(##__VA_ARGS__)
#define NS_EVENT_ASYNC(...) NS_EVENT.async(##__VA_ARGS__)
#define NS_CPU_NUM NS_EVENT.cpu_num()

#endif
