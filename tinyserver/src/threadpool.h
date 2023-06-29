#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<list>
#include<cstdio>
#include<exception>
#include<pthread.h>
#include"locker.h"
#include"sql_connection_pool.h"
template<class T>
class threadpool
{
public:
    //thread_number是线程池中线程的数量，max_requsts是请求队列中最多允许的，等待处理的请求的数量
    threadpool(int act_model,connection_pool* connPool,int thread_number=8,int max_request=10000);
    ~threadpool();
    bool append(T* request,int state);
    bool append_p(T* request);
private:
    static void* worker(void* args);
    void run();
private:
    int m_thread_number;//线程池中的线程数；
    int m_max_requests;//请求队列中允许的最大请求数；
    pthread_t* m_threads;//描述线程池大小的数组，其大小为m_thread_number;
    std::list<T*> m_workqueue;//请求队列；
    locker m_queuelocker;
    sem m_queuestat;//信号量，是否有数据需要处理；
    connnection_pool* m_connPool;//数据库；
    int m_actor_model;//模型切换；

};
template<class T>
threadpool<T>::threadpool(int act_model,connection_pool* connPool,int thread_number,int max_requests)
:m_actor_model(actor_model),m_thread_number(thread_number),m_max_requests(max_requests),m_threads(NULL),m_connPool(connPool)
{
    if(thread_number<=0||max_requests<=0)
    {
        throw std::exception();
    }
    m_threads=new pthread_t[m_thread_number];
    if(!m_threads)
    {
        throw std::exception();
    }
    for(int i=0;i<thread_number;++i)
    {
        if(pthread_create(m_threads+1,NULL,worker,this)!=0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
        }
    }
}
template<typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}
template<typename T>
bool threadpool<T>::append(T* request,int state)
{
    m_queuelocker.lock();
    if(m_workqueue.size()>=m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return ture;
}
template<typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool=(threadpool*)arg;
    pool->run();
    return pool;
}
template<typename T>
void threadpool<T>::run()
{
    while (true)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request=m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
            continue;
        if(1==m_actor_model)
        {
            if(0==request->m_state)
            {
                if(request->read_once())
                {
                    request->improv=1;
                    connectionRAII mysqlcon(&request->mysql,m_connPool);
                    request->process;
                }
                else{
                    request->improv=1;
                    request->time_flag=1;
                }
            }
            else{
                if(request->write())
                {
                    request->improv=1;
                }
                else{
                    request->improv=1;
                    request->timer_flag=1;
                }
            }

        }
        else{
            connectionRAII mysqlcon(&request->mysql,m_connPool);
            request->process();
        }
    }
    
}









#endif