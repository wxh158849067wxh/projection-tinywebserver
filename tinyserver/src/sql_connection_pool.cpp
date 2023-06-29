#include"sql_connection_pool.h"
#include<pthread.h>
#include<stdlib.h>

connection_pool::connection_pool()
{
    m_curconn=0;
    m_freeconn=0;
}
//单例模式构造
connection_pool* connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}
void connection_pool::init(std::string url,std::string user,std::string passward,std::string dbname,int port,int maxconn,int close_log)
{
    m_url=url;
    m_port=port;
    m_user=user;
    m_password=passward;
    m_database_name=dbname;
    m_close_log=close_log;
    for(int i=0;i<maxconn;++i)
    {
        MYSQL* con=NULL;
        con =mysql_init(con);
        if(con==NULL)
        {
            LOG_ERROR("MYSQL ERROR!");
            exit(1);
        }
        con=mysql_real_connect(con,url.c_str(),user.c_str(),passward.c_str(),dbname.c_str(),port,NULL,0);
        if(con==NULL)
        {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }
        connList.push_back(con);
        ++m_freeconn;


    }
    reserver=sem(m_freeconn);
    m_maxconn=m_freeconn;
}
//返回时需要判断是否为NULL
MYSQL* connection_pool::GetConnection()
{
   MYSQL *con=NULL;
   if(0==connList.size())
   {
    return NULL;
   } 
   reserver.wait();//如果连接池没有可用连接则阻塞等待
   lock.lock();//先上锁,线程安全
   con=connList.front();
   connList.pop_front();
   --m_freeconn;
   ++m_curconn;
   lock.unlock()//解锁
   return con;
}
//通过返回值判断是否成果
bool connection_pool::ReleaseConnection(MYSQL*) 
{
    if(NULL==con)
    {
        return false;
    }
    lock.lock();//上锁，互斥操作
    connList.push_back(con);//将free了的连接重新放回资源池
    ++m_freeconn;
    --m_curconn; 
    lock.unlock();
    reserver.post();//信号量加一
    return true;
}
void connection_pool::DestroyPool()
{
    lock.lock();
    if(connList.size()>0)
    {
        std::list<MYSQL*>::iterator:: it;
        for(it =connList.begin();it!=connList.end();++it)
        {
            MYSQL* con=*it;
            mysql_close(con);
        }
        m_curconn=0;
        m_freeconn=0;
        connList.clear();

    }
    lock.unlock();
}
//返回当前空余连接数
int connection_pool::GetFreeConn()
{
    return this->m_freeconn;
}
connection_pool::~connection_pool()
{
    DestroyPool();
}

connectionRAII::connectionRAII(MYSQL** SQL,connection_pool* connPool)
{
    *SQL=connPool->GetConnection();
    conRAII=*SQL;
    poolRAII=connPool;
}
connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection(conRAII);
}


