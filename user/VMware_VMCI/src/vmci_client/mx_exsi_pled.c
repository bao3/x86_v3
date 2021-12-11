#include <sys/types.h>
/*
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <sys/un.h>
#include <unistd.h>
#endif
#include <errno.h>
#include "vmci_sockets.h"
#include "device_control.h"	/* Define the packaet format */

void usage(char *name) {
	printf("Get/set the programmable LED utility\n");
	printf("Usage: %s -l -n [-r|-w] [-g|-s] [-h]\n", name);
	printf("    Show the mx_exsi_pled information if no argument apply.\n");
	printf("    -h: Show this information.\n");
	printf("    -l: List the number of LEDs.\n");
	printf("    -n: Indicate the n-the LED.\n");
	printf("    -r: Read the LED bitmap.\n");
	printf("    -w: Write the bitmap to the LED.\n");
	printf("    -g: Get the LED port value.\n");
	printf("    -s: Set the value to the LED port.\n");
}
#ifdef WIN32
extern int
getopt(int nargc, char *const *nargv, const char *ostr);
#endif
extern int optind, opterr, optopt;
extern char *optarg;

/* entry point */
int main(int argc, char *argv[])
{
	int fd, result;
	int num;
	int value = 0;
	int nth = 0;
	int bWrite = 0;
	int bRead = 0;
	int bSet = 0;
	int bGet = 0;
	int bList = 0;
	char led_bitmap[]="00000000";
	char optstring[] = "hn:s:gw:rl";
	char c;

	if ( argc == 1 ) {
		usage(argv[0]);
		return 0;
	}

	while ((c = getopt(argc, argv, optstring)) != -1)
		switch (c) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'n':
			nth=atoi(optarg);
			if ( nth <=0 || nth >8 ) {
				printf(" nth:%d is not in 1 and 8\n", nth);
				return 0;
			}
			break;
		case 's':
			bSet = 1; 
			value=atoi(optarg);
			break;
		case 'g':
			bGet = 1;
			break;
		case 'w':
			bWrite = 1;
			strcpy(led_bitmap, optarg);
			break;
		case 'r':
			bRead = 1;
			break;
		case 'l':
			bList = 1;
			break;
		case '?':
			printf("Invalid option\n");
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 0;
		}


	fd = device_open("pled");
	if ( fd < 0 ) {
		printf("device_open() fail\n");
		goto main_close;
	}
	

	if ( ( nth >= 1 && nth <= 8 ) && bGet == 0 && bSet == 0 ) {
		printf("The -n option should be used with -g or -s\n");
		printf("EX: To set the first led on by, `mx_exsi_pled -n 1 -s 1`\n");
		goto main_close;
	}
	else if ( bList == 1 ) {
		num = device_list(fd);
		if ( num < 0 ) {
			printf("device_list() fail\n");
			goto main_close;
		}
		printf("pled number:%d\n", num);
	}
	else if ( bGet == 1 ) {
		result = device_get(fd, nth, &value);
		printf("Get pled[%d] value %d\n", nth, value);
		if ( result < 0 ) {
			printf("device_get() fail\n");
			goto main_close;
		}
	}
	else if ( bSet == 1 ) {
		if ( value == 0 ) {
			printf("Turn off the LED, %d\n", nth);
			result = device_set(fd, nth, 0);
		}
		else {
			printf("Turn on the LED, %d\n", nth);
			result = device_set(fd, nth, 1);
		}
		if ( result < 0 ) {
			printf("device_set() fail\n");
			goto main_close;
		}
	}
	else if ( bRead == 1 ) {
		result = device_read(fd, led_bitmap, sizeof(led_bitmap));
		if ( result < 0 ) {
			printf("device_read() fail, result:%d\n", result);
			goto main_close;
		}
		printf(" The led_bitmap is %s\n", led_bitmap);
	}
	else if ( bWrite == 1 ) {
		/* The size of led_bitmap should include the '\0' or '\n' */
		result = device_write(fd, led_bitmap, strlen(led_bitmap)+1);
		if ( result < 0 ) {
			printf("device_write() fail\n");
			goto main_close;
		}
		printf("Write the LED bitmap:%s\n", led_bitmap);
	}

main_close:

	device_close(fd);

	return 0 ;
}
