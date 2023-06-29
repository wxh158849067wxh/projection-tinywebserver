#include"lst_timer.h"
#include"http_conn.h"

sort_timer_lst::sort_timer_lst()
{
    head=NULL;
    tail=NULL;
}
sort_timer_lst::~sort_timer_lst()
{
    util_timer tmp=head;
    while (tmp!=tail)
    {
        head=tmp->next;
        delete tmp;
        tmp=head;    
    }
    
}
void sort_timer_lst::add_timer(util_timer* timer)
{
    if(!timer)
    {
        return;
    }
    if(!head)//当链表为空时
    {
        head=tail=timer;
        return;
    }
    if(timer->expire<head->expire)//如果新增的timer先于链表首个timer先过期，则将新增timer置于链表首
    {
        timer->next=head;
        head->prev=timer;
        head=timer;
        return;
    }
    add_timer(timer,head);
}
void sort_timer_lst::adjust_timer(util_timer* timer)
{
    if(!timer)
    return;
    util_timer* tmp=timer->next;//处理timer为tail时
    if(!tmp||(timer->expire<tmp->expire))//
    {
        return;
    }
    if(timer==head)//处理timer为head的情况
    {
        head=head->next;
        head->prev=NULL;
        timer->next=NULL;
        add_timer(timer,head);
    }
    else{//处理timer不为tail和head的情况
        timer->prev->next=timer->next;
        timer->next->prev=timer->prev;
        add_timer(timer,timer->next)
    }
}
void sort_timer_lst::del_timer(util_timer* timer)
{
    if(!timer)//timer为空直接退出
    {
        return;
    }
    if((timer==head)&&(timer==tail))//当list只有一个util_timer
    {
        delete timer;
        head=NULL;
        tail=NULL;
        return;
    }
    if(timer==head)//timer是list头节点
    {
        util_timer* tmp=head;
        head=head->next;
        head->prev=NULL;
        delete tmp;
        return;
    }
    if(timer==tail)//timer是list尾节点
    {
        tail->prev->next=NULL;
        tail=tail->prev;
        delete timer;
        return;
    }
    //timer在链表中间
    timer->prev->next=timer->next;
    timer->next->prev=timer->prev;
    delete timer;
}
void sort_timer_lst::tick()
{
    if(!head)
    {
        return;
    }
    time_t cur=time(NULL);//获取标准时间以来的秒数
    util_timer* tmp=head;
    while(tmp)
    {
        if(cur<tmp->expire)//当前链表第一个还未过期，expire是timer过期的时间，从1900标准时间起算的时间
        {
            break;
        }
        tmp->cb_func(tmp->user_data);
        head=tmp->next;
        if(head)
        {
            head->prev=NULL;//将当前的prev置空
        }
        delete tmp;//删除当前被处理的timer
        tmp=head;//将当前指针指向下一个
    }
}
void sort_timer_lst::add_timer(util_timer* timer,util_timer* lst_head)//timer的下一个即为lst_head
{
    util_timer* prev=lst_head;
    util_timer* tmp=prev->next;
    while(tmp)
    {
        if(timer->expire<tmp->expire)//timer先于head的next过期则将timer插入
        {
            prev->next=timer;
            timer->next=tmp;
            tmp->prev=timer;
            timer->prev=prev;
            break;
        }
        prev=tmp;
        tmp=tmp->next;
    }
    if(!tmp)//如果知道链表尾timer都未被插入，说明tmp到达链表尾，pre为tail，则将timer插入到链表尾
    {
        prev->nex=timer;
        timer->prev=prev;
        timer->next=NULL;
        tail=timer;
    }

}
void Utils::init(int timeslot)
{
    m_TIMESLOT=timeslot;
}
//将文件描述符设置为非阻塞
void Utils::setnonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}
void Utils::addfd(int epollfd,int fd,bool one_shot,int TRIGMode)
{
    epoll_event event;
    event.data.fd=fd;
    if(1==TRIGMode)
    {
        event.events=EPOLLIN|EPOLLET|EPOLLRDHUP;
    }
    else
    {
        event.events=EPOLLIN|EPOLLRDHUP;
    }
    if(one_shot)
    {
        event.events|=EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}
//信号处理函数
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno=errno;
    int msg=sig;
    send(u_pipefd[1],(char*)&msg,1,0);
    errno=save_errno;
}

void Utils::addsig(int sig,void(handle)(int),bool restart)
{
    struct sigaction sa;
}
