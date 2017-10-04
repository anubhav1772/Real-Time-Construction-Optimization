#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <iostream>

using namespace std;
class ThreadPool;
class Worker
 {
   private:
     ThreadPool &pool;
   public:
     Worker(ThreadPool &s) : pool(s) { }
     void operator()();
 };
class ThreadPool
 {
   private:
     friend class Worker;
     vector< thread > workers;
     deque< function<void()> > tasks;
     mutex queue_mutex;
     condition_variable condition;
     bool stop;
   public:
     ThreadPool(size_t);
     template<class F>
     void enqueue(F f);
     ~ThreadPool();

 };
void Worker::operator()()
{
     function<void()> task;
     while(true)
        {
            {
                unique_lock<mutex>
                lock(pool.queue_mutex);
                while(!pool.stop && pool.tasks.empty())
                    {
                        pool.condition.wait(lock);
                    }
                if(pool.stop)
                    return;
                task = pool.tasks.front();
                pool.tasks.pop_front();
            }
            task();
        }
}
ThreadPool::ThreadPool(size_t threads):stop(false)
{
        for(size_t i=0;i<threads;i++)
        {

            workers.push_back(thread(Worker(*this)));
        }
}
ThreadPool::~ThreadPool()
{
        stop = true;
        condition.notify_all();
        for(size_t i = 0;i<workers.size();++i)
            workers[i].join();
}
template<class F>
void ThreadPool::enqueue(F f)
{
     {
           unique_lock<mutex> lock(queue_mutex);
           tasks.push_back(function<void()>(f));
     }
     condition.notify_one();
}
int main()
{
    ThreadPool pool(4);
    for(int i = 0;i<8;++i)
    {
       pool.enqueue([i]
       {
         cout << "Thread " << i << endl;
         this_thread::sleep_for(chrono::seconds(1));
         cout << "Pool " << i << endl;
       });
    }
    return 0;
}
