#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<sys/stat.h>
#include<string>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<stdarg.h>
#include<error.h>
#include<sys/wait.h>
#include<sys/uio.h>
#include<map>


#include"locker.h"
#include"sql_connection_pool.h"
#include "lst_timer.h"
#include"log.h"
class http_conn
{
public:
    static const int FILENAME_LEN=200;
    static const int READ_BUFFER_SIZE=2048;
    static const int WRITE_BUFFER_SIZE=1024;
    enum METHOD
    {
        GET=0;
        POST;
        HEAD;
        PUT;
        DELETE;
        TRACE;
        OPTIONS;
        CONNECT;
        PATH;
    };
    enum HTTP_CODE
    {
        NO_REQUEST;
        GET_REQUEST;
        BAD_REQUEST;
        NO_RESOURCE;
        FORBIDDEN_REQUEST;
        FILE_REQUEST;
        INTERNAL_ERROR;
        CLOSED_CONNECTION;
    };
    enum LINE_STATUS
    {
        LINE_OK=O;
        LINE_BAD;
        LINE_OPEN;
    };
public:
    http_conn();
    ~http_conn();
public: 
    void init(int sockfd,const sockaddr_in& addr,char* ,int ,int,string user,string password,string sqlname);
    void close_conn(bool real_close=true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in* get_address();
    {
        return &m_address;
    }
    void initmysql_result(connection_pool* connPool);
    int timer_flag;
    int improv;
private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_contend(char* text);
    HTTP_CODE do_request();
    char* get_line()
    {
        return m_read_buf+m_start_line;
    }
    LINE_STATUS parse_line();
    void ummap();
    bool add_response(const char* format,...);
    bool add_content(const char* content);
    bool add_status_line(int status,const char* title);
    bool add_headers(int content_length);
    bool add_linger();
    bool add_blank_line();
public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL* mysql;
    int m_states;//读为0，写为1；
    
private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];



};



















#endif