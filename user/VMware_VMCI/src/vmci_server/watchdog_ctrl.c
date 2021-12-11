#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/watchdog.h>

#define MX_WATCHDOG "/dev/watchdog"

int watchdog_get(int index, int *value) {
	int ret = 0, watchdog_fd;

	watchdog_fd = open(MX_WATCHDOG, O_RDWR);
	if ( watchdog_fd < 0 ) {
		perror("watchdog_open open() fail");
		*value = -1;
		return watchdog_fd;
	}

	ret = ioctl(watchdog_fd, WDIOC_GETTIMEOUT, value);
	if( ret < 0 ) {
		perror("WDIOC_GETTIMEOUT fail\n");
		*value = -1;
		close(watchdog_fd);
		return ret;
	}
#ifdef DEBUG
	printf("The watchdog count is %d\n", *value );
#endif

	close(watchdog_fd);

	return ret;
}

int watchdog_set(int index, int value) {
	int ret=0, watchdog_fd;

	watchdog_fd = open(MX_WATCHDOG, O_RDWR);
	if ( watchdog_fd < 0 ) {
		perror("watchdog_set open() fail\n");
		close(watchdog_fd);
		return watchdog_fd;
	}

	ret = ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &value);
	if ( ret < 0 ) {
		perror("pled write() fail");
		close(watchdog_fd);
		return ret;
	}

	close(watchdog_fd);

	return ret;
}

int watchdog_read(int size, char *data) {
	int ret=0, watchdog_fd;

	watchdog_fd = open(MX_WATCHDOG, O_RDWR);
	if ( watchdog_fd < 0 ) {
		perror("watchdog_set open() fail\n");
		*data = -1;
		return watchdog_fd;
	}

	ret = ioctl(watchdog_fd, WDIOC_GETTIMEOUT, data);
	if( ret < 0 ) {
		perror("WDIOC_GETTIMEOUT fail\n");
		*data = -1;
		close(watchdog_fd);
		return ret;
	}

	close(watchdog_fd);

	return ret;
}

int watchdog_write(int size, char *data) {
	int ret=0, watchdog_fd;

	watchdog_fd = open(MX_WATCHDOG, O_RDWR);
	if ( watchdog_fd < 0 ) {
		perror("watchdog_write open() fail\n");
		*data = -1;
		return watchdog_fd;
	}

	ret = ioctl(watchdog_fd, WDIOC_SETTIMEOUT, data);
	if ( ret < 0 ) {
		perror("pled write() fail");
		close(watchdog_fd);
		return ret;
	}

	close(watchdog_fd);

	return ret;
}
