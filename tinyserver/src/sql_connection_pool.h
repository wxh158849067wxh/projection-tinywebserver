#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include<iostream>
#include<list>
#include<mysql.h>
#include<error.h>
#include<string>
#include<stdio.h>
#include"locker.h"
#include"log.h"

class connection_pool
{
public:
    MYSQL* GetConnection();//获取数据库连接
    bool ReleaseConnection(MYSQL* conn);//释放连接
    int GetFreeConn();//获取连接
    void DestroyPool();


   //单例模式
    static connection_pool* GetInstance();
    void init(std::string url,std::string User,std::string Password, std::strng DataBaseName,int port,int MaxConn,int close_log);
private:
    connection_pool();
    ~connection_pool();

    int m_maxconn;//最大连接数；
    int m_curconn;//当前使用的连接数
    int m_freeconn;//当前空闲的连接数
    locker lock;
    std::list<MYSQL*> connList;//连接池
    sem reserver;
public:
    std::string m_url;
    std::string m_port;
    std::string m_user;
    std::string m_password;
    std::string m_database_name;
    std::string m_close_log;

};
class connectionRAII{
public:
    connectionRAII();
    ~connectionRAII();
private:
    MYSQL *conRAII;
    connection_pool* poolRAII;

   
};


#endif