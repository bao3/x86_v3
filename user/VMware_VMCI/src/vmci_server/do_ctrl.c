#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
//#include "do_ctl.h"

#define MX_DIO_NAME "/dev/dio"

#define MX_DI_MINOR 0
#define IOCTL_SET_DOUT	_IOW(MX_DI_MINOR,15,int)
#define IOCTL_GET_DOUT	_IOW(MX_DI_MINOR,16,int)
#define IOCTL_GET_DIN	_IOW(MX_DI_MINOR,17,int)
#define IOCTL_SET_DIN	_IOW(MX_DI_MINOR,18,int) /* Not supported */

typedef struct dio_struct {
	int	port;
	int	data;
} dio_t;

int do_get(int index, int *value) {
	int ret=0, fd;
	dio_t dio;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("do open() fail");
		*value = -1;
		return fd;
	}

	dio.port = index;
	ret = ioctl(fd, IOCTL_GET_DOUT, &dio);
	if( ret < 0 ) {
		perror("IOCTL_GET_DOUT fail\n");
		*value = -1;
		close(fd);
		return ret;
	}

	*value = dio.data;
#ifdef DEBUG
	printf("The DO is %s\n", (*value) ? "1" : "0" );
#endif

	close(fd);

	return ret;
}

int do_set(int index, int value) {
	int ret=0, fd;
	dio_t dio;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("do open() fail");
		return fd;
	}

	/* Turn on/off the DO */
	dio.port = index;
	dio.port = value;
	ret = ioctl(fd, IOCTL_SET_DOUT, &dio);
	if( ret < 0 ) {
		perror("do ioctl() fail");
		close(fd);
		return ret;
	}

	close(fd);

	return ret;
}

int do_read(int size, char *data) {
	int ret=0, fd;
	dio_t dio;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("do_read open() fail");
		*data = -1;
		return ret;
	}

	dio.port = -1;
	ret = ioctl(fd, IOCTL_GET_DOUT, &dio);
	if( ret < 0 ) {
		perror("do_read read() fail");
		*data = -1;
		close(fd);
		return ret;
	}

	close(fd);
	*data = dio.data;

	return ret;
}

int do_write(int size, char *data) {
	int ret=0;
	/* The DOUT not supports write pattern */

	*data = 0;
	return ret;
}
