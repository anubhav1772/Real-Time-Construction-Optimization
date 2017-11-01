#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

using namespace std;

class ThreadPool;

class Brick
{
private:
    int id;      /* task with priority id=0 masontask*/
    string color;
    int length;
    int breadth;
    int width;
public:
    Brick(int priority_id,string col,int l,int b,int h):id(priority_id),color(col),length(l),breadth(b),width(h){ }
};

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
    friend class Worker;    /* Now worker class can access the private members of ThreadPool class */
    vector<thread> workers; /* need to keep track of threads so we can join them */
    deque<Brick> masonTasks;
    deque<Brick> labourTasks;
    mutex mut;
    condition_variable condition;
    bool stop;
public:
    ThreadPool(size_t);
    template<class F>
    void enqueue(F f);
    ~ThreadPool();
};

ThreadPool::ThreadPool(size_t threads):stop(false)
{
        for(size_t i=0;i<threads;i++) /* threads is basically the count of number of live threads in the pool*/
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

void ThreadPool::enqueue(Brick brick)
{
     {
           unique_lock<mutex> lock(mut);
           if(brick.id==0)
           {
               masonTasks.push_back(brick);
           }
           else                            /* in case priority id is 1 */
           {
               labourTasks.push_back(brick);
           }
     }
     condition.notify_one();
}

int main()
{
    ThreadPool pool(10);
    int no_of_jobs=20;
    for(int i=1;i<=20;i++)
    {


    }

}
