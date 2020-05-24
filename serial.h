#ifndef SERIAL_H_
#define SERIAL_H_

int serial_data_avail(int fd);
int serial_open(const char* serialName,int nSpeed);
int serial_close(int fd);
long serial_read(int fd, char *buf, int len);
int serial_write(int fd, char *buf, int len);
int serial_flush(int fd);

#endif

