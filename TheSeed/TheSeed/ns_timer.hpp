/*
    ��ʱ��

*/
#ifndef NS_TIMER_H_
#define NS_TIMER_H_
#include <ctime>
#include "event.hpp"
namespace ns
{
    namespace event
    {
        typedef long timer_id;
        typedef clock_t timer_ms;
        typedef std::function<void(timer_id id)> timer_out_handler;

        //��ʱ��
        struct Timer
        {
            //��ʱ��id
            timer_id id;
            //��ʱʱ��
            timer_ms time_out_ms;
            //�������
            timer_out_handler handler;

        };
        typedef std::shared_ptr<Timer> shared_timer;

        class NSTimer
        {
        public:
            //����
            static NSTimer& instance()
            {
                static NSTimer ts;
                return ts;

            }
        public:
            //��Ӽ�ʱ��
            timer_id add_timer(timer_ms timeout, timer_out_handler handler)
            {
                if (timeout <= 0 || !handler)
                    return -1;

                auto t = std::make_shared<Timer>();
                t->time_out_ms = clock() + timeout;
                if (t->time_out_ms < 0)
                {
                    printf("t->time_out_ms < 0\n");
                    return -1;
                }
                t->handler = handler;

                {
                    std::unique_lock<std::mutex> lock(timer_map_lock_);
                    t->id = round_id_;
                    timer_map_id_.emplace(t->id, t);
                    timer_map_ms_.emplace(t->time_out_ms, t);
                    ++round_id_;
                }
                return t->id;
                //Timer
            }
            //�Ƴ���ʱ��
            void del_timer(timer_id id)
            {
                std::unique_lock<std::mutex> lock(timer_map_lock_);
                //timer_map_.erase(id);
                auto t = timer_map_id_.find(id);
                if (t == timer_map_id_.end())
                    return;

                auto p = timer_map_ms_.equal_range(t->second->time_out_ms);
                for (auto ptr = p.first; ptr != p.second; ++ptr)
                {
                    if (ptr->second->id = id)
                    {
                        timer_map_ms_.erase(ptr);
                        break;
                    }
                }
                timer_map_id_.erase(t);

                //timer_ma
            }

            void set_timer_check(timer_ms ms)
            {
                timer_check_ = ms;
            }

            ~NSTimer()
            {
                exit_flag_ = true;
                if (timer_update_thread_.joinable())
                {
                    timer_update_thread_.join();
                }
            }
        private:
            NSTimer() :round_id_(0), last_clock_(0), timer_check_(1)
            {
                exit_flag_ = false;
                
                std::thread t([this]() {
                    while (!exit_flag_)
                    {
                        try
                        {
                            updata_time(timer_check_);
                        }
                        catch (std::exception & e)
                        {
                            printf("[Event] updata_time err,msg:%s\n", e.what());
                        }
                    }
                    });

                timer_update_thread_.swap(t);
            }
            //�����
            void updata_time(timer_ms check_time_ms = 5)
            {
                timer_ms sleep_time = check_time_ms;
                //����ʱ��
                {
                    std::unique_lock<std::mutex> lock(timer_map_lock_);
                    auto now = clock();
                    if (last_clock_ != 0)
                        sleep_time -= (now - last_clock_);
                    last_clock_ = now;
                    //ȡ����ʱ�¼�
                    auto pend = timer_map_ms_.upper_bound(now);
                    auto p = timer_map_ms_.begin();
                    while (p != pend)
                    {
                        auto t = p->second;
                        NS_EVENT_ASYNC_VOID(t->handler, t->id);
                        //t->handler(t->id);
                        p = timer_map_ms_.erase(p);
                        timer_map_id_.erase(t->id);

                    }
                }

                if (sleep_time > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

                // async_void(&Event::updata_time, this, check_time_ms);
            }
        private:
           
            std::atomic_bool exit_flag_;

            //��ʱ���߳�
            //��
            std::mutex timer_map_lock_;
            //��ʱid ����
            timer_id round_id_;
            //�ϴθ���ʱ��
            timer_ms last_clock_;
            //ʱ���������
            timer_ms timer_check_;
            //�����߳�
            std::thread timer_update_thread_;
            //��ʱ������
            std::unordered_map<timer_id, shared_timer> timer_map_id_;
            std::multimap<timer_ms, shared_timer> timer_map_ms_;

        };
    }

}
#define NS_TIMER ns::event::NSTimer::instance()
#define NS_ADD_TIMER(ms,handler) NS_TIMER.add_timer(ms,handler)
#define NS_DEL_TIMER(id) NS_TIMER.del_timer(id)
#endif