#include<string>
#include<time.h>
#include<sys/time.h>
#include<stdarg.h>
#include"log.h"
#include<pthread.h>

Log::log()
{
    m_count=0;
    m_is_async=false;
}
~Log::Log()
{
    if(m_fp!=NULL)
    {
        fclose(fp);
    }
}
bool Log::init(const char* file_name,int close_log,int log_buf_size,int split_line,int max_queue_size)
{
    //if max_queue_size is set, then state is asynchronous
    if(max_size_size>=1)
    {
        m_is_async=true;
        m_log_queue =new block_queue<std::string>(max_queue_size);
        pthread_t tid;
        //flush_log_thread is callback function,there create a 
        //pthread to write logs into file asynchronously
        pthread_create(&tid,NUll,flush_log_thread,NULL);
    }
    m_close_log=close_log;
    m_log_buf_size=log_buf_size;
    m_buf=new char [m_log_buf_size];
    memset(m_buf,'\0',m_log_buf_size);
    m_split_line=split_line;
    
    time_t t=time(NULL);
    struct tm* sys_tm=localtime(&t);
    struct tm my_tm=*sys_tm;
    
    const char* p=strrchr(file_name,'/');
    char log_full_name[255]={0};
    if(p==NULL)
    {
        snprintf(log_full_name,256,"%d_%02d_%02d_%s",my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,file_name);
    }
    else{
        strcpy(log_name,p+1);
        strncpy(dir_name,file_name,p-file_name+1);
        snprintf(log_full_name,255,"%s%d_%02d_%02d_%s",dir_name,my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,log_name);

    }
    m_today=my_tm.tm_mday;
    m_fp=fopen(log_full_name,"a");
    if(m_fp==NULL)
    {
        return false;
    }
    return true;
}