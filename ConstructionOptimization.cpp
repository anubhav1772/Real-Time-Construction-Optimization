#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

using namespace std;

class ThreadPool;

// In real life construction, task of mason is to build while task of labourer is to carry brick and help mason.
class Brick
{

    //int id;      /* task with priority id=0 masontask */
public:
    string color;
    int length;
    int breadth;
    int width;
    Brick(string col,int l,int b,int h):color(col),length(l),breadth(b),width(h){ }
    //void print_details();
};

/*void Brick::print_details()
{
    cout<<"Color: "<<color<<" Length: "<<length<<" Breadth: "<<breadth<<" Height: "<<width<<endl;
}*/

//worker threads' class
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
    //friend class Brick;
    vector<thread> workers; /* need to keep track of threads so that we can join them */
    deque< function<void()> > masonTasks;
    deque< function<void()> > labourTasks;
    mutex mut;
    condition_variable condition;
    bool stop; //stop is true when task is completed.
public:
    ThreadPool(size_t);
    template<class F>
    void enqueue(int priority,F f);
    ~ThreadPool();
};

void Worker::operator()()
{
     function<void()> task;
     while(true)
        {
            {
                unique_lock<mutex> lock(pool.mut);
                while(!pool.stop && pool.masonTasks.empty())
                    {
                        pool.condition.wait(lock);
                    }
                if(pool.stop)
                    return;
                task = pool.masonTasks.front();
                pool.masonTasks.pop_front();
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
			 masonTasks.push_back(function<void()>(f));
		 }
		 condition.notify_one();
	 }
	 else if(priority==1)
	 {
		 labourTasks.push_back(function<void()>(f));
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
       Brick b1("Blue",10,5,5);
	   pool.enqueue(1,[b1]
       {
		  cout<<"Work done by Labourer. ";
		 // b1.print_details();
		  cout<<"Color: "<<b1.color<<" Length: "<<b1.length<<" Breadth: "<<b1.breadth<<" Height: "<<b1.width<<endl;

	   });
	   this_thread::sleep_for(chrono::seconds(1));
	   Brick b2("Red",10,5,5);
	   pool.enqueue(0,[b2]{
		  cout<<"Work done by mason. ";
		 // b2.print_details();
		  cout<<"Color: "<<b2.color<<" Length: "<<b2.length<<" Breadth: "<<b2.breadth<<" Height: "<<b2.width<<endl;
	   });


    }
    return 0;

}
