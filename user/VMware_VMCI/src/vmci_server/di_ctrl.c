#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
//#include "di_ctl.h"

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

int di_get(int index, int *value) {
	int ret=0, fd;
	dio_t dio;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("di open() fail");
		*value = -1;
		return fd;
	}

	dio.port = index;
	ret = ioctl(fd, IOCTL_GET_DIN, &dio);
	if( ret < 0 ) {
		perror("IOCTL_GET_DIN fail\n");
		*value = -1;
		close(fd);
		return ret;
	}

#ifdef DEBUG
	printf("The DI is %s\n", (*value) ? "1" : "0" );
#endif
	*value = dio.data;
	close(fd);

	return ret;
}

int di_set(int index, int value) {
	int ret=0;

	/* The DI cannot be set, just return success but not do any setting. */

	return ret;	
}

int di_read(int size, char *data) {
	int ret=0, fd;
	dio_t dio;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("di_read open() fail");
		*data = -1;
		return ret;
	}

	dio.port = -1;
	ret = ioctl(fd, IOCTL_GET_DIN, &dio);
	if( ret < 0 ) {
		perror("di_read read() fail");
		*data = -1;
		close(fd);
		return ret;
	}

	close(fd);
	*data = dio.data;

	return ret;
}

int di_write(int size, char *data) {
	int ret=0;

	/* The DI cannot be written, just return success but not do any setting. */

	return ret;
}
