#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

//#include "pled_ctrl.h"

int get_ledbitmap_nth_value(int n, char bitmap[]) {
	return ( bitmap[n] == '0' ) ? 0 : 1;
}

int pled_get(int index, int *value) {
	int ret=0, led_fd;
	char return_led_bitmap[]="00000000"; /* All the LED initial status is off */

	led_fd = open("/dev/pled", O_RDWR);
	if ( led_fd < 0 ) {
		*value = -1;
		return ret;
	}

	ret = read(led_fd, return_led_bitmap, sizeof(return_led_bitmap));
	if( ret < 0 ) {
		*value = -1;
	}

	*value = get_ledbitmap_nth_value(index-1, return_led_bitmap);

	close(led_fd);
	return ret;
}

void turn_on_nth_ledbitmap(int n, char bitmap[]) {
	bitmap[n]='1';
}

void turn_off_nth_ledbitmap(int n, char bitmap[]) {
	bitmap[n]='0';
}

int pled_set(int index, int value) {
	int ret=0, led_fd;
	char set_led_bitmap[]="00000000"; /* All the LED initial status is off */

	led_fd = open("/dev/pled", O_RDWR);
	if ( led_fd < 0 ) {
		perror("pled open() fail");
		return led_fd;
	}

	ret = read(led_fd, set_led_bitmap, sizeof(set_led_bitmap));
	if( ret < 0 ) {
		perror("pled read() fail");
		close(led_fd);
		return ret;
	}

	/* Turn on/off the nth bit in led_bitmap */
	if ( value == 0 )
		turn_off_nth_ledbitmap(index-1, set_led_bitmap);
	else
		turn_on_nth_ledbitmap(index-1, set_led_bitmap);
	
	/* Use the led_bitmap to control the LED */
	ret = write(led_fd, set_led_bitmap, sizeof(set_led_bitmap));
	if ( ret < 0 ) {
		perror("pled write() fail");
	}

	close(led_fd);

	return ret;	
}

int pled_read(int size, char *data) {
	int ret=0, led_fd;

	led_fd = open("/dev/pled", O_RDWR);
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

int pled_write(int size, char *data) {
	int ret=0, led_fd;

	led_fd = open("/dev/pled", O_RDWR);
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
