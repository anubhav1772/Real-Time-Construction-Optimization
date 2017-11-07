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

    int id;      /* task with priority id=0 masontask */
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
    //deque<Brick> masonTasks;
    //deque<Brick> labourTasks;
	deque< function<void(Brick)> > masonTasks;
	deque< function<void(Brick)> > labourTasks;
    mutex mut;
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
     function<void(Brick)> task;
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
		{
			workers[i].join();
		}
            
}

template<class F>
void ThreadPool::enqueue(int priority,F f)
{
     if(priority==0)
	 {
		 {
			 unique_lock<mutex> lock(mut);
			 masonTasks.push_back(function<void(Brick)>(f));
		 }
		 condition.notify_one(); 
	 }
	 else if(priority==1)
	 {
		 labourTasks.push_back(function<void(Brick)>(f));
		 if(masonTasks.empty())
		 {
			 unique_lock<mutex> lock(mut);
			 masonTasks.push_back(labourTasks.front());
			 labourTasks.pop_front();
		 }
		 condition.notify_one();
	 }
}

int main()
{
    ThreadPool pool(10);/* worker threads=10 */
    int no_of_task=20;/* mason task + labour task */
    for(int i=0;i<no_of_task/2;i++)
    {
       // pool.enqueue(new Brick(0,"RED",10,5,5));
       // pool.enqueue(new Brick(1,"BLUE",10,5,5));
	   
	   pool.enqueue(1,[](Brick b(1,"Blue",10,5,5)){
		  cout<<"Work done by Labourer. ";
		  cout<<"Color: "<<b.color<<" Length: "<<b.length<<" Breadth: "<<b.breadth<<" Height: "<<b.width<<endl;
	   });
	   this_thread::sleep_for(chrono::seconds(1));
	   pool.enqueue(0,[](Brick b(0,"RED",10,5,5)){
		  cout<<"Work done by mason. ";
		  cout<<"Color: "<<b.color<<" Length: "<<b.length<<" Breadth: "<<b.breadth<<" Height: "<<b.width<<endl;
	   });
	   
	   
    }
    return 0;

}
