#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
//#include "do_ctl.h"

#define MX_DIO_NAME "/dev/dio"

#define MX_DI_MINOR 0
#if 0
#define IOCTL_SET_DOUT	_IOW(MX_DI_MINOR,15,int)
#define IOCTL_GET_DOUT	_IOW(MX_DI_MINOR,16,int)
#define IOCTL_GET_DIN	_IOW(MX_DI_MINOR,17,int)
#define IOCTL_GET_RELAY	_IOW(MX_DI_MINOR,18,int) /* Not supported */
#define IOCTL_SET_RELAY	_IOW(MX_DI_MINOR,19,int)
#else
#define IOCTL_SET_DOUT	15
#define IOCTL_GET_DOUT	16
#define IOCTL_GET_DIN	17
#define IOCTL_GET_RELAY	18 /* Not supported */
#define IOCTL_SET_RELAY	19
#endif

typedef struct dio_struct {
	int	port;
	int	data;
} dio_t;

int relay_get(int index, int *value) {
	int ret=0, fd;
	dio_t relay;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("do open() fail");
		*value = -1;
		return fd;
	}
	relay.port = index;
	ret = ioctl(fd, IOCTL_GET_RELAY, &relay);
	if( ret < 0 ) {
		perror("IOCTL_GET_RELAY fail\n");
		*value = -1;
		close(fd);
		return ret;
	}
	#if 1
	*value = relay.data;
	#else
	*value = ret;
	#endif
#ifdef DEBUG
	printf("The relay is %s\n", (*value) ? "1" : "0" );
#endif

	close(fd);

	return ret;
}

int relay_set(int index, int value) {
	int ret=0, fd;
	dio_t relay;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("do open() fail");
		return fd;
	}

	/* Turn on/off the DO */
	#if 1
	relay.port = index;
	relay.data = value;
	ret = ioctl(fd, IOCTL_SET_RELAY, &relay);
	#else
	if (value)
		ret = ioctl(fd, IOCTL_SET_RELAY, 1);
	else
		ret = ioctl(fd, IOCTL_SET_RELAY, 0);
	#endif
	if( ret < 0 ) {
		perror("do ioctl() fail");
		close(fd);
		return ret;
	}

	close(fd);

	return ret;
}

int relay_read(int size, char *data) {
	int ret = 0, fd;
	dio_t relay;

#if 0	/* To do: it's not supported. */
	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("relay_read open() fail");
		*data = -1;
		return ret;
	}

	relay.port = -1;
	ret = ioctl(fd, IOCTL_GET_RELAY, &relay);
	if( ret < 0 ) {
		perror("relay_read read() fail");
		*data = -1;
		close(fd);
		return ret;
	}

	close(fd);
	*data = relay.data;
#endif
	return ret;
}

int relay_write(int size, char *data) {
	int ret=0;
	/* The DOUT not supports write pattern */

	*data = 0;
	return ret;
}
