#include <fcntl.h>  
#include <errno.h>  
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include "serial.h"
#include "buffer.h"

struct termios serial_oldtio;

static int set_serial_opts(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	int ret = -1;
	struct termios serial_newtio;
	
	if (fd < 0)
		return -1;
	
	if (tcgetattr(fd, &serial_oldtio) != 0)
	{
		goto fail;
	}
	
	memset(&serial_newtio, 0, sizeof(serial_newtio));
			
	serial_newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	serial_newtio.c_oflag &= ~OPOST;
	serial_newtio.c_iflag &= ~(ICRNL | IGNCR);
	serial_newtio.c_cflag |= CLOCAL | CREAD;
	serial_newtio.c_cflag &= ~CSIZE;

	serial_newtio.c_cc[VTIME] = 0; 
	serial_newtio.c_cc[VMIN]  = 0; 
	
	serial_newtio.c_cc[VINTR] = 0; 
	serial_newtio.c_cc[VQUIT] = 0; 
	serial_newtio.c_cc[VERASE] = 0; 
	serial_newtio.c_cc[VKILL] = 0; 
	serial_newtio.c_cc[VEOF] = 0; 
	serial_newtio.c_cc[VTIME] = 1; 
	serial_newtio.c_cc[VMIN] = 0; 
	serial_newtio.c_cc[VSWTC] = 0; 
	serial_newtio.c_cc[VSTART] = 0; 
	serial_newtio.c_cc[VSTOP] = 0; 
	serial_newtio.c_cc[VSUSP] = 0; 
	serial_newtio.c_cc[VEOL] = 0; 
	serial_newtio.c_cc[VREPRINT] = 0; 
	serial_newtio.c_cc[VDISCARD] = 0; 
	serial_newtio.c_cc[VWERASE] = 0; 
	serial_newtio.c_cc[VLNEXT] = 0; 
	serial_newtio.c_cc[VEOL2] = 0; 
	
	switch (nBits)
	{
		case 7: serial_newtio.c_cflag |= CS7;break;
		case 8: serial_newtio.c_cflag |= CS8;break;
		default:
			serial_newtio.c_cflag |= CS8;break;
	}
	
	
	switch (nEvent)
	{
		case 'O':
			serial_newtio.c_cflag |= PARENB;
			serial_newtio.c_cflag |= PARODD;
			serial_newtio.c_iflag |= (INPCK|ISTRIP);
			break;
			
		case 'E':
			serial_newtio.c_cflag |= PARENB;
			serial_newtio.c_cflag &= ~PARODD;
			serial_newtio.c_iflag |= (INPCK|ISTRIP);
			break;
		
		case 'N':
			serial_newtio.c_cflag &= ~PARENB;
			break;
		
		default:
			serial_newtio.c_cflag &= ~PARENB;
			break;
	}
	
	switch (nSpeed)
	{
		case 9600:
			cfsetispeed(&serial_newtio, B9600);
			cfsetospeed(&serial_newtio, B9600);
			break;
		
		case 19200:
			cfsetispeed(&serial_newtio, B19200);
			cfsetospeed(&serial_newtio, B19200);
			break;
			
		case 38400:
			cfsetispeed(&serial_newtio, B38400);
			cfsetospeed(&serial_newtio, B38400);
			break;
		
		case 57600:
			cfsetispeed(&serial_newtio, B57600);
			cfsetospeed(&serial_newtio, B57600);
			break;
			
		case 115200:
			cfsetispeed(&serial_newtio, B115200);
			cfsetospeed(&serial_newtio, B115200);
			break;

		case 115200*2:
			cfsetispeed(&serial_newtio, B230400);
			cfsetospeed(&serial_newtio, B230400);
			break;

		case 921600:
			cfsetispeed(&serial_newtio,B921600);
			cfsetospeed(&serial_newtio,B921600);
			break;
		default:
			cfsetispeed(&serial_newtio, B9600);
			cfsetospeed(&serial_newtio, B9600);
			break;
	}
	
	if (nStop == 1)
		serial_newtio.c_cflag &= ~CSTOPB;
	else if (nStop == 2)
		serial_newtio.c_cflag = CSTOPB;
		
	tcflush(fd, TCIFLUSH);
			
	if (tcsetattr(fd, TCSANOW, &serial_newtio) != 0)
	{
		goto fail;
	}
	
	ret = 0;
			
	fail:
		return ret;
}
 
int serial_open(const char* serialName, int nSpeed)
{
	int fd = open(serialName,O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	//int fd = open(serialName,O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd > 0)
	{
		if (set_serial_opts(fd, nSpeed, 8, 'N', 1) == -1)
		{

			LOGE("set_serial_opts Errors\n");
			return -1;
		}
	}
	return fd;
}

int serial_close(int fd)
{
	if (fd < 0)
		return -1;
		
	tcsetattr(fd, TCSANOW, &serial_oldtio);
		
	if (!close(fd))
	{
		return 0;
	}	
	
	return -1;
}


int serial_write(int fd, char *buf, int len)
{
	int ret = -1;
	
	if (fd < 0)
		return ret;
	
	ret = write(fd, buf, len);
	
	if(ret <= 0)
	{
		printf("ret = %d, errno = %d\n", ret, errno);
	}
	return ret;
}

int serial_data_avail(int fd)
{
  int result;

  if (ioctl (fd, FIONREAD, &result) == -1)
    return -1;

  return result;
}

long serial_read(int fd, char *buf, int len)
{
	long ret = -1;
	ret = read(fd, buf, len);
	return ret;
}


int serial_flush(int fd)
{
	  int nread = 0;
		int try_egain = 0;
		char rev_buffer[MAX_READ_LEN_BUFF] = {0};
	  
		usleep(200000);
		tcflush(fd, TCIOFLUSH);
		
		return 0;
}


