/*
	�¼�ϵͳ

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
			//����
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

            // ����µ������������
            template<class F, class... Args>
            auto async(F&& f, Args&& ... args)
                -> std::future<typename std::result_of<F(Args...)>::type>
            {
                // ��ȡ��������ֵ����        
                using return_type = typename std::result_of<F(Args...)>::type;

                // ����һ��ָ�������ֻ��ָ��
                auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                std::future<return_type> res = task->get_future();
                //����
                post([task]() { (*task)(); });
                return res;
            }

            // ����µ���������
            template<class F, class... Args>
            auto async_first(F&& f, Args&& ... args)
                -> std::future<typename std::result_of<F(Args...)>::type>
            {
                // ��ȡ��������ֵ����        
                using return_type = typename std::result_of<F(Args...)>::type;

                // ����һ��ָ�������ֻ��ָ��
                auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                std::future<return_type> res = task->get_future();
                //����
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
                    // task��һ���������ͣ���������н�������
                    task_func task;

                    {
                       // printf("worker is lock!\n");
                        //���������������������������ڽ������Զ�����
                        std::unique_lock<std::mutex> lock(list_lock_);

                        //��1����������������falseʱ�������̣߳�����ʱ�Զ��ͷ�����
                        //��2����������������true���ܵ�֪ͨʱ��������Ȼ�������
                        list_condition_.wait(lock, \
                            [this] 
                            {
                                //���˳�
                                if (exit_flag_)
                                    return true;
                                //δ�˳� �� ��������Ϊ�� ��������
                                if (!task_list_.empty()&&run_flag_)
                                    return true;
                                return false;
                            });

                        if (exit_flag_ && task_list_.empty())
                            break;

                        //���������ȡ��һ������
                        task = std::move(task_list_.front());
                        task_list_.pop_front();
                      //  printf("worker is unlock!\n");
                        //printf("[Event] task_list_ size %d\n", task_list_.size());
                    }                           
                    // �Զ�����
                    //printf("test in \n");
                    try
                    {
                       // printf("[Event] run task in\n");
                        task();                      // ִ���������
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
			//�����߳�
			std::vector<run_thread_item> run_thread_vector_;
			
            std::atomic_bool exit_flag_;
            std::atomic_bool run_flag_;

            //�����б�
            //��
            std::mutex list_lock_;
            //�������� 
            std::condition_variable list_condition_;             
            //�������
            std::list<task_func> task_list_;

		};
	}

}

#define NS_EVENT ns::event::Event::instance()
#define NS_EVENT_ASYNC_VOID(...) NS_EVENT.async_void(##__VA_ARGS__)
#define NS_EVENT_ASYNC(...) NS_EVENT.async(##__VA_ARGS__)
#define NS_CPU_NUM NS_EVENT.cpu_num()

#endif
