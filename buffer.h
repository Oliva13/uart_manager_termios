#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_READ_LEN_BUFF 512

#define MOD_UART_MANAGER "uart_manager"
#define SIPMANAGER  "SIP"
#define INTERCOMAPI "INTERCOMAPI"

#define LOGE printf

extern pthread_mutex_t  gmtx;

typedef struct 
{
    int epfd;
    int pd;
    int uart_fd;
    char pipe_name[32];

}th_param_t;

void init_pthrd(th_param_t *p);
void close_pthrd(th_param_t *p);
void on_sigterm(int pd);

#endif
