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
#include "buffer.h"
#include "result.h"

void printf_result_pthread(result_t result)
{
    switch(result)
    {
          case OPEN_EPOOL_WAIT_ERROR: 
          {
               printf("OPEN_EPOOL_WAIT_ERROR\n");     
          }
          break;
          case EPOOL_WORK_ERROR:
          {
               printf("EPOOL_WORK_ERROR\n");     
          }
          break;
          case EPOOL_WORK_DONE:
          {
               printf("EPOOL_WORK_DONE\n");     
          }
          break;
          case READ_UART_EAGAIN:
          {
                printf("READ_UART_EAGAIN\n");     
          }
          break;
          case READ_UART_ERROR:
          {
                printf("READ_UART_ERROR\n");     
          }
          break;
          case READ_PIPE_ERROR:
          {
                printf("READ_PIPE_ERROR\n");     
          }
          break;
	      case WRITE_UART_ERROR:
          {
                printf("WRITE_UART_ERROR\n");     
          }
          break;
	      case WRITE_UART_EAGAIN:
          {
                printf("WRITE_UART_EAGAIN\n");     
          }
          break;
          default: 
          {
              printf("undefine error - %d\n", result);     
          } 
          break;
    }
}

void printf_result_main(result_t result, th_param_t *p)
{
       switch(result)
       {
          case MAIN_EPOOL_CREATE_ERROR: 
          {
               printf("MAIN_EPOOL_CREATE_ERROR\n");     
          }
          break;
	        case MAIN_MKFIFO_CREATE_ERROR: 
          {
                printf("MAIN_MKFIFO_CREATE_ERROR\n");
          }
          break;
	        case MAIN_PIPE_OPEN_ERROR: 
          {
                printf("MAIN_PIPE_OPEN_ERROR\n");
                unlink(p->pipe_name);
          }
          break;
	        case MAIN_ADD_PIPE_TO_EPFD_ERROR: 
          {
                printf("MAIN_ADD_PIPE_TO_EPFD_ERROR\n");
                unlink(p->pipe_name);
                close(p->pd);
          }
          break;
	       case MAIN_SERIAL_PORT_ERROR: 
          {
                printf("MAIN_SERIAL_PORT_ERROR\n");
                unlink(p->pipe_name);
                close(p->pd);
          } 
          break;
	        case MAIN_ADD_UART_TO_EPFD_ERROR: 
          {
                printf("MAIN_ADD_UART_TO_EPFD_ERROR\n");
                unlink(p->pipe_name);
                close(p->pd);
				serial_flush(p->uart_fd);
                serial_close(p->uart_fd);
				
          }
          break;
	        case  MAIN_CREATE_PTHREAD_ERROR: 
          {
                printf("MAIN_CREATE_PTHREAD_ERROR\n");
                unlink(p->pipe_name);
                close(p->pd);
				serial_flush(p->uart_fd);
                serial_close(p->uart_fd);
          }
          break; 
          default: 
          {
              printf("undefine error - %d\n", result);     
          } 
          break;
    }
}
