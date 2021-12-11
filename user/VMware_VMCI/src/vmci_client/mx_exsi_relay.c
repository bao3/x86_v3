#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#ifndef WIN32
#include <sys/ioctl.h>
#else
#include "vmci_sockets.h"
#include "device_control.h" /* Define the packaet format */
#endif

#define MIN_RELAY 1
#define MAX_RELAY 1

void usage(char *name) {
	printf("Get/Set the RELAY status utility\n");
	printf("Usage: %s [-g|-s] [-h]\n", name);
	printf("    -g Get the n-th RELAY status.\n");
	printf("    -s: Set the value to the RELAY.\n");
	printf("    -h: Show this information\n");
}


#ifdef WIN32
extern int
getopt(int nargc, char *const *nargv, const char *ostr);
#endif
extern int optind, opterr, optopt;
extern char *optarg;

int main(int argc, char *argv[]) {
	int	c ;
	int	fd_relay;
	int	value = 0;
	int	nth = 1; // The relay is at 0x305 bit 5
	int	bSet = 0;
	int	bGet = 0;
	int	result = 0;
	char	optstring[] = "s:gh";

	if ( argc == 1 ) {
		usage(argv[0]);
		return 0;
	}
	
	while ((c = getopt(argc, argv, optstring)) != -1)
		switch (c) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			bSet = 1; 
			value=atoi(optarg);
			break;
		case 'g':
			bGet = 1;
			break;
		default:
			usage(argv[0]);
			return 0;
		}

	fd_relay = device_open("relay");
	if ( fd_relay < 0 ) {
		printf("device_open() fail\n");
		goto main_close;
	}

	if ( bGet == 1 ) {
		result = device_get(fd_relay, nth, &value);
		printf("Get relay value %d\n", value);
		if ( result < 0 ) {
			printf("device_list() fail\n");
			goto main_close;
		}
	}
	else if ( bSet == 1 ) {
		if ( value == 0 ) {
			printf("Set 0 to relay\n");
			result = device_set(fd_relay, nth, 0);
		}
		else {
			printf("Set 1 to relay\n");
			result = device_set(fd_relay, nth, 1);
		}
		if ( result < 0 ) {
			printf("device_set() fail\n");
			goto main_close;
		}
	}

main_close:
	device_close(fd_relay);

	return 0;
}
