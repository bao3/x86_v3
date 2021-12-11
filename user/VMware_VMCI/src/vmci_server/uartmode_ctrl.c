#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

//#include "uartmode_ctl.h"

#define MX_DIO_NAME "/dev/dio"

#define MX_DI_MINOR 0
#define IOCTL_SET_DOUT	_IOW(MX_DI_MINOR,15,int)
#define IOCTL_GET_DOUT	_IOW(MX_DI_MINOR,16,int)
#define IOCTL_GET_DIN	_IOW(MX_DI_MINOR,17,int)
#define IOCTL_SET_DIN	_IOW(MX_DI_MINOR,18,int) /* Not supported */

#define MOXA	0x400
#define MOXA_SET_OP_MODE        (MOXA + 66)
#define MOXA_GET_OP_MODE        (MOXA + 67)
#define RS232_MODE              0
#define RS485_2WIRE_MODE        1
#define RS422_MODE              2
#define RS485_4WIRE_MODE        3
#define NOT_SET_MODE		4

typedef struct dio_struct {
	int	port;
	int	data;
} dio_t;


int uartmode_get(int index, int *value) {
	int ret=0, fd;
	unsigned char opmode;
	dio_t dio;

	fd = open(MX_DIO_NAME, O_RDWR);
	if ( fd < 0 ) {
		perror("uartmode open() fail");
		*value = -1;
		return ret;
	}

	dio.port = index;
	ret = ioctl(fd, MOXA_GET_OP_MODE, &dio);
	if( ret < 0 ) {
		perror("IOCTL_GET_DIN fail\n");
		*value = -1;
		close(fd);
		return ret;
	}

	close(led_fd);
	return ret;
}

int uartmode_set(int index, int value) {
	int ret=0, led_fd;
	char set_led_bitmap[]="00000000"; /* All the LED initial status is off */

	led_fd = open("/dev/dio", O_RDWR);
	if ( led_fd < 0 ) {
		perror("dio open() fail");
		return led_fd;
	}

	ret = read(led_fd, set_led_bitmap, sizeof(set_led_bitmap));
	if( ret < 0 ) {
		perror("dio read() fail");
		close(led_fd);
		return ret;
	}

	/* Use the led_bitmap to control the LED */
	ret = write(led_fd, set_led_bitmap, sizeof(set_led_bitmap));
	if ( ret < 0 ) {
		perror("dio write() fail");
	}

	close(led_fd);

	return ret;	
}

int uartmode_read(int size, char *data) {
	int ret=0, led_fd;

	led_fd = open("/dev/dio", O_RDWR);
	if ( led_fd < 0 ) {
		*data = -1;
		return ret;
	}

	ret = read(led_fd, data, size);
	if( ret < 0 ) {
		*data = -1;
	}

	close(led_fd);
	return ret;
}

int uartmode_write(int size, char *data) {
	int ret=0, led_fd;

	led_fd = open("/dev/dio", O_RDWR);
	if ( led_fd < 0 ) {
		*data = -1;
		return ret;
	}

	ret = write(led_fd, data, size);
	if( ret < 0 ) {
		*data = -1;
	}

	close(led_fd);
	return ret;
}
