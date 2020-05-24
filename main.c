#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include "serial.h"
#include "result.h"
#include "buffer.h"

#define MAX_EVENTS     2

th_param_t  th_param;
static int  done = 0;
pthread_mutex_t  gmtx;

void init_pthrd(th_param_t *p)
{
    p->epfd = -1;
    p->pd = -1;
    p->uart_fd = -1;
    pthread_mutex_init(&gmtx, NULL);
	done = 0;
}

void close_pthrd(th_param_t *p)
{
	done = 1;
    p->epfd = -1;
    p->pd = -1;
    p->uart_fd = -1;
	pthread_mutex_destroy(&gmtx);
}

static void *threadFunc(void *arg)
{
    result_t result = PARSE_SUCCESS;
    th_param_t *p = (th_param_t *)arg;
    struct epoll_event evlist[MAX_EVENTS];
    long tnum = (long)arg;
    int ret, i = 0;
    char rev_buffer[MAX_READ_LEN_BUFF] = {0};
    int nread = 0;
    int try_again = 0;

    while(1)
    {
        ret = epoll_wait(p->epfd, evlist, MAX_EVENTS, 500);
        if (ret < 0)
        {
            printf("Error: epoll_wait\n");
		    result = OPEN_EPOOL_WAIT_ERROR;
            goto Error;
        }  

        if((evlist[i].events & EPOLLERR) || (evlist[i].events & EPOLLHUP)) 
        {
              printf("epoll error\n");
              result = EPOOL_WORK_ERROR;
              goto Error;
        }

        if(!ret)
        {
            pthread_mutex_lock(&gmtx);
    	    if(done)
    	    {
           	    pthread_mutex_unlock(&gmtx);
                result = EPOOL_WORK_DONE;
		        goto Error;
	        }
	        pthread_mutex_unlock(&gmtx);
            continue;
        }

    	//printf("ret - %d\n", ret);

        for(i = 0; i < ret; i++)
        {
            if(evlist[i].events & EPOLLIN)
            {   
     	        int temp_fd = evlist[i].data.fd;
                if(temp_fd == p->uart_fd)
                {	
					nread = read(temp_fd, rev_buffer, sizeof(rev_buffer)-2);
					if(nread < 0)
					{    
						if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK && errno != 0)
						{
							printf("1. Error: read data to uart...errno - %d\n", errno);
                            result = READ_UART_ERROR;
							goto Error;
						}
						else 
						{
                            printf("2. Error: read data to uart...errno - %d\n", errno);
                            result = READ_UART_EAGAIN;
                            goto Error;
						}
					}
					
                    rev_buffer[nread] = '\0';
                    					
					printf("read data from uart :%s, bytes - %d\n", rev_buffer, nread);
                    nread = 0; 
                    //
			        // передаем данные, полученные из uart, киентам
			        //
			        //ipc_send(SIPMANAGER,  rev_buffer);
			        //ipc_send(INTERCOMAPI, rev_buffer);
                }

                if(temp_fd == p->pd)
                {
                    nread = read(temp_fd, rev_buffer, sizeof(rev_buffer)-2);
    	            if(nread < 0)
    	            {    
        		        printf("Error: read data from pipe...\n");
                        result = READ_PIPE_ERROR;
                        goto Error;
                    }
                    rev_buffer[nread] = '\0';
                    printf("read data from pipe: %s, bytes - %d\n", rev_buffer, nread);
                    nread = 0;

                    size_t rem =  strlen(rev_buffer);
                	while(rem > 0)
	                {
		                nread = serial_write(p->uart_fd, rev_buffer, rem);
		                if(nread < 0)
                        {
			                  if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK && errno != 0)
			                  {
				                    printf("Error: write data to uart...errno - %d\n", errno);
                                    result = WRITE_UART_ERROR;
                                    goto Error;
			                  }
			                  else 
                              {
                                    if(try_again < 20)
                                    {
                                        try_again++;
                                        continue;
                                    }
                                    else
                                    {
                                        result = WRITE_UART_EAGAIN;
                                        goto Error;
                                    }
                                    
                              }         
		                }
                		rem -= nread;
                    }
                    printf("Write to uart: %s, bytes - %d\n", rev_buffer, nread);
                    nread = 0; 
                    try_again = 0;
                }
            }
        }
   }
   printf("Thread %ld completed epoll_wait(); ready = %d\n", tnum, ret);

Error:
  
   pthread_exit((void *)result);
   return NULL;
}

int add_fd_to_epfd(int epfd, int fd,  uint32_t ev_flags)
{
    struct epoll_event event;
    int flags;
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    event.events = ev_flags; // | EPOLLET;
    event.data.fd = fd;

    if( epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event))
    {
	    printf("epoll_ctl(ADD) error\n");
        return 1;
    }
    else
    {
       return 0;
    }
}

void on_sigterm(int pd)
{
	pthread_mutex_lock(&gmtx);
	done = 1;
	pthread_mutex_unlock(&gmtx);
}
   
int main(int argc, char *argv[])
{
    int s = -1;
    pthread_t t1;
   
    int ret = 0;
    result_t result_main = PARSE_SUCCESS;
    result_t *result_pthread;
    char   *serialName = "/dev/ttyACM0";
    int    nSpeed = 921600;

    signal(SIGINT, on_sigterm);
    signal(SIGTERM, on_sigterm);

    init_pthrd(&th_param);

    th_param_t *p = &th_param;

    p->uart_fd = serial_open(serialName, nSpeed);
    if(p->uart_fd < 0)
    {
	    serialName = "/dev/ttyACM1";
	    p->uart_fd = serial_open(serialName, nSpeed);
	    if(p->uart_fd < 0)
	    {
    		LOGE("1 error serial_open - %s\n", serialName);
            result_main = MAIN_SERIAL_PORT_ERROR;
            goto exit;
	    }
    }

    serial_flush(p->uart_fd);

    p->epfd = epoll_create(MAX_EVENTS);
    if (p->epfd == -1)
    {
        printf("epoll_create\n");
        result_main = MAIN_EPOOL_CREATE_ERROR;
        goto exit;
    }

    sprintf(p->pipe_name, "%s", "/tvh/mod/uart_manager");

    if (mkfifo(p->pipe_name, 0666) < 0)
    {
	    printf("%s registering failed (pipe not created)", p->pipe_name);
        result_main = MAIN_MKFIFO_CREATE_ERROR;
	    goto exit;
    }

    p->pd = open(p->pipe_name, O_RDWR|O_NONBLOCK);
    if (p->pd < 0)
    {
	    printf("%s registering failed (pipe cannot be opened)", p->pipe_name);
        result_main = MAIN_PIPE_OPEN_ERROR;
	    goto exit;
    }

    ret = add_fd_to_epfd(p->epfd, p->pd, EPOLLIN);    
    if(ret < 0)
    {
        printf("Error: pipe add_fd_to_epfd - %d\n", p->pd);
        result_main = MAIN_ADD_PIPE_TO_EPFD_ERROR;
	    goto exit;
    }
    
    ret = add_fd_to_epfd(p->epfd, p->uart_fd, EPOLLIN);
    if(ret < 0)
    {
        printf("Error: uart add_fd_to_epfd - %d\n", p->uart_fd);
        result_main = MAIN_ADD_UART_TO_EPFD_ERROR;
	    goto exit;
    }

    s = pthread_create(&t1, NULL, threadFunc, (void *)&th_param);
    if (s != 0)
    {
        result_main = MAIN_CREATE_PTHREAD_ERROR;
	    goto exit;
    }

    pthread_join(t1, (void *)&result_pthread);

    //printf("result from pthread - %d\n", (int)result_pthread);
    
    printf_result_pthread((result_t)result_pthread); 
    
    close(p->pd);
    unlink(p->pipe_name);

    serial_flush(p->uart_fd);
    ret = serial_close(p->uart_fd);
    if(ret < 0)
    {	
    	printf("serial_close\n");
    	return 1;
    }
    close_pthrd(&th_param);
    return 0;

exit: 
    printf_result_main(result_main, &th_param);
    return 1;
}

