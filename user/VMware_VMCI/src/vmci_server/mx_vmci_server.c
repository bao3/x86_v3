#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
/* Semaphore, for accept(), read() mutual exclusion */
#include <semaphore.h>
#include "vmci_sockets.h"
#include "device_control.h"	/* Define the packaet format */

#define DA820_PLED	0
#define DA820_RELAY	1	/* For relay */
#define DA820_DI	2	/* For DI */
#define DA820_DO	3	/* For DO */
#define DA820_UARTMODE	4	/* For RS-232/422/485 configure */
#if 0 /* TODO: The watchdog has not supported now */
#define DA820_WATCHDOG	5
#endif

typedef struct __devices_list__ {
	char device_type[32];	/* pled, relay, DI, DO, ... */
	int number;	/* Total number of devices */
	int fd; /* Set fd to 1 if deviced has opened  */
	int (*device_get)(int, int*);
	int (*device_set)(int, int);
	int (*device_read)(int, char*);
	int (*device_write)(int, char*);
} devices_list;

/* Hash the file descriptor value */
#define GET_INDEX_I(FILE_DESCRIPTOR) (FILE_DESCRIPTOR/10)
#define GET_INDEX_J(FILE_DESCRIPTOR) ((FILE_DESCRIPTOR%10)-1)


int lookup_device_index(char *device_type);
int do_device_list(device_control *pRequestPkt, device_control *pReturnPkt);
int do_device_open(device_control *pRequestPkt, device_control *pReturnPkt);
int do_device_close(device_control *pRequestPkt, device_control *pReturnPkt);
int do_device_get(device_control *pRequestPkt, device_control *pReturnPkt);
int do_device_set(device_control *pRequestPkt, device_control *pReturnPkt);
int do_device_read(device_control *pRequestPkt, device_control *pReturnPkt);
int do_device_write(device_control *pRequestPkt, device_control *pReturnPkt);
int parse_and_dispatch(device_control *pPkt, device_control *pReturnPkt);

/* Declare the devices supported on the ESXi server */
devices_list DA820[] = {
	{"pled", 8, -1, pled_get, pled_set, pled_read, pled_write},
	{"relay", 1, -1, relay_get, relay_set, relay_read, relay_write},
#if 0 /* TODO: The DI/DO has not supported now */
	{"dio", 6, -1, di_get, di_set, di_read, di_write},
	{"do", 2, -1, do_get, do_set, do_read, do_write},
	{"uartmode", 2, -1, uartmode_get, uartmode_set, uartmode_read, uartmode_write},
#endif
#if 0 /* TODO: The watchdog has not supported now */
	{"watchdog", 1, -1, watchdog_get, watchdog_set, watchdog_read, watchdog_write},
#endif
};

int lookup_device_index(char *device_type) {
	int i;

	/* Return the index for the device_type */
	for (i=0; i< sizeof(DA820)/sizeof(devices_list); i++)
		if( strncmp(device_type, DA820[i].device_type, strlen(device_type)) == 0 )
			return i;

	return -E_DEVTYPE_NOT_SUPPORT;
}

int do_device_list(device_control *pRequestPkt, device_control *pReturnPkt) {
	int i, fd;
	int index_i;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	fd = atoi(pRequestPkt->device_type);

	index_i = GET_INDEX_I(fd);

	if ( index_i < 0 || index_i >= (sizeof(DA820)/sizeof(devices_list)) ) {
		printf("do_device_list(): the device, %d, is not supported\n", index_i);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	if ( DA820[index_i].fd < 0 ) {
		printf("do_device_list(): the file descriptor, DA820[%d].fd:%d, has been released\n", index_i, DA820[index_i].fd);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_DEVICE_NOT_OPENED;
	}

	/* The number of supported devices */
	sprintf(pReturnPkt->arg1, "%d", DA820[index_i].number);

	return 0;
}

int do_device_open(device_control *pRequestPkt, device_control *pReturnPkt) {
	int i;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	i = lookup_device_index(pRequestPkt->device_type);

	if ( i < 0 ) {
		printf("do_device_open(): the device, %s, is not supported\n", pRequestPkt->device_type);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	/* Check if the fd is unused (-1) */
	if ( DA820[i].fd == -1 ) {
		/* File descriptor assignment */
		DA820[i].fd= (i*10) + 1 ;
	}

	sprintf(pReturnPkt->arg1, "%d", DA820[i].fd);

	return DA820[i].fd;
}

int do_device_close(device_control *pRequestPkt, device_control *pReturnPkt) {
	int fd;
	int index_i;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	fd = atoi(pRequestPkt->device_type);

	index_i = GET_INDEX_I(fd);

	if ( index_i < 0 || index_i >= (sizeof(DA820)/sizeof(devices_list)) ) {
		printf("do_device_close(): the device, %d, is not supported\n", index_i);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	if ( DA820[index_i].fd < 0 ) {
		printf("do_device_close(): the file descriptor has been released\n");
		sprintf(pReturnPkt->arg1, "%d", -E_DEVICE_NOT_OPENED); 
		return -E_DEVICE_NOT_OPENED;
	}

	/* Release the file descriptor */
	DA820[index_i].fd = -1;

	return 0;
}

int do_device_get(device_control *pRequestPkt, device_control *pReturnPkt) {
	int fd, retval=0;
	int index, value;
	int index_i;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	fd = atoi(pRequestPkt->device_type);

	index_i = GET_INDEX_I(fd);

	if ( index_i < 0 || index_i >= (sizeof(DA820)/sizeof(devices_list)) ) {
		printf("do_device_get(): the device, %d, is not supported\n", index_i);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	if ( DA820[index_i].fd < 0 ) {
		printf("do_device_get(): the fd:%d has not opened\n", DA820[index_i].fd);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_DEVICE_NOT_OPENED;
	}

	index = atoi(pRequestPkt->arg1);
	/* The index should start from 1, ... */
	if ( index <= 0 || index > DA820[index_i].number ) {
		printf("do_device_get(): the index, %d, is not available\n", index);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_INDEX_NOT_AVAILABLE;
	}

	/* Get the device_value */
	retval = DA820[index_i].device_get(index, &value);
	if( retval < 0 ) {
		printf("DA820[%d].device_get() fail\n", index_i);
		return retval;
	}

	/* Return the value */
	sprintf(pReturnPkt->arg1, "%d", value);

	return 0;
}

int do_device_set(device_control *pRequestPkt, device_control *pReturnPkt) {
	int fd, retval=0;
	int index, value;
	int index_i;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	fd = atoi(pRequestPkt->device_type);

	index_i = GET_INDEX_I(fd);

	if ( index_i < 0 || index_i >= (sizeof(DA820)/sizeof(devices_list)) ) {
		printf("do_device_set(): the device, %d, is not supported\n", index_i);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	if ( DA820[index_i].fd < 0 ) {
		printf("do_device_set(): the fd:%d has not opened\n", DA820[index_i].fd);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_DEVICE_NOT_OPENED;
	}

	index = atoi(pRequestPkt->arg1);
	/* The index should start from 1, ... */
	if ( index <= 0 || index > DA820[index_i].number ) {
		printf("do_device_set(): the index, %d, is not available\n", index);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_INDEX_NOT_AVAILABLE;
	}

	value = atoi(pRequestPkt->arg2);
	if ( value < 0 ) {
		printf("do_device_set(): the value, %d, is invalid\n", value);
		sprintf(pReturnPkt->arg1, "%d", -E_VALUE_INVALID); 
		return -E_VALUE_INVALID;
	}

	/* Set the device_value */
	retval = DA820[index_i].device_set(index, value);
	if( retval < 0 ) {
		printf("DA820[%d].device_set() fail\n", index_i);
		return retval;
	}

	/* Return the value */
	sprintf(pReturnPkt->arg1, "%d", value);

	return 0;
}

int do_device_read(device_control *pRequestPkt, device_control *pReturnPkt) {
	int fd, retval=0;
	int index_i, size;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	fd = atoi(pRequestPkt->device_type);

	index_i = GET_INDEX_I(fd);

	if ( index_i < 0 || index_i >= (sizeof(DA820)/sizeof(devices_list)) ) {
		printf("do_device_read(): the device, %d, is not supported\n", index_i);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	if ( DA820[index_i].fd < 0 ) {
		printf("do_device_read(): the fd:%d has not opened\n", DA820[index_i].fd);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_DEVICE_NOT_OPENED;
	}

	/* Read the data from the device */
	size = atoi(pRequestPkt->arg1);
	retval = DA820[index_i].device_read(size, pReturnPkt->arg2);
	if( retval < 0 ) {
		printf("DA820[%d].device.read() fail\n", index_i);
		return retval;
	}

	/* Return the value */
	sprintf(pReturnPkt->arg1, "%d", retval);

	return retval;
}

int do_device_write(device_control *pRequestPkt, device_control *pReturnPkt) {
	int fd, retval=0;
	int index_i, size;

	memset(pReturnPkt, 0, sizeof(device_control));
	strcpy(pReturnPkt->device_request, pRequestPkt->device_request);
	strcpy(pReturnPkt->device_type, pRequestPkt->device_type);

	fd = atoi(pRequestPkt->device_type);

	index_i = GET_INDEX_I(fd);

	if ( index_i < 0 || index_i >= (sizeof(DA820)/sizeof(devices_list)) ) {
		printf("do_device_write(): the device, %d, is not supported\n", index_i);
		sprintf(pReturnPkt->arg1, "%d", -E_DEVTYPE_NOT_SUPPORT); 
		return -E_DEVTYPE_NOT_SUPPORT;
	}

	if ( DA820[index_i].fd < 0 ) {
		printf("do_device_write(): the fd:%d has not opened\n", DA820[index_i].fd);
		sprintf(pReturnPkt->arg1, "%d", -E_INDEX_NOT_AVAILABLE); 
		return -E_DEVICE_NOT_OPENED;
	}

	/* Write the data to the device */
	size = atoi(pRequestPkt->arg1);
	retval = DA820[index_i].device_write(size, pRequestPkt->arg2);
	if( retval < 0 ) {
		printf("DA820[%d].device.write() fail\n", index_i);
		return retval;
	}
	strcpy(pReturnPkt->arg2, pRequestPkt->arg2);

	/* Return the value */
	sprintf(pReturnPkt->arg1, "%d", retval);

	return retval;
}

int parse_and_dispatch(device_control *pPkt, device_control *pReturnPkt) {
	int retval = 0;

	if ( strcmp(pPkt->device_request, DEVICE_LIST_REQUEST) == 0 ) {
		retval = do_device_list(pPkt, pReturnPkt);
	}
	else if ( strcmp(pPkt->device_request, DEVICE_OPEN_REQUEST) == 0 ) {
		retval = do_device_open(pPkt, pReturnPkt);
	}
	else if ( strcmp(pPkt->device_request, DEVICE_CLOSE_REQUEST) == 0 ) {
		retval = do_device_close(pPkt, pReturnPkt);
	}
	else if ( strcmp(pPkt->device_request, DEVICE_GET_REQUEST) == 0 ) {
		retval = do_device_get(pPkt, pReturnPkt);
	}
	else if ( strcmp(pPkt->device_request, DEVICE_SET_REQUEST) == 0 ) {
		retval = do_device_set(pPkt, pReturnPkt);
	}
	else if ( strcmp(pPkt->device_request, DEVICE_WRITE_REQUEST) == 0 ) {
		retval = do_device_write(pPkt, pReturnPkt);
	}
	else if ( strcmp(pPkt->device_request, DEVICE_READ_REQUEST) == 0 ) {
		retval = do_device_read(pPkt, pReturnPkt);
	}
	else {
		return  -E_REQ_NOT_SUPPORT;
	}

	return retval;
}

device_control pkt, ret_pkt;

int main(int argc, char *argv[])
{
	int result;
	int client_len ;
	int max_fd=0;
	fd_set	read_fdset;
	struct timeval lease_time = {
		.tv_sec = 1L,
		.tv_usec = 0
	};
	vmci_socket vsock;
	/* Semaphore, for accept(), read() mutual exclusion */
	int pshared=0;
	sem_t sem;

	sem_init(&sem, pshared, 1);

	result = init_server_socket(&vsock);
	if( result < 0 )
		goto main_close;

	FD_ZERO(&read_fdset);

	int count=0;
	while(1) {

		//FD_ZERO(&read_fdset);
		FD_SET(vsock.server_sockfd, &read_fdset);
		if ( vsock.server_sockfd > max_fd )
			max_fd = vsock.server_sockfd;

	/* 2021/03/24, Jared */
	/* It fails in ESXi 6.x in select() system call with a small delay. So I modify it to always block at select(). */
	#ifndef ALWAYS_BLOCKED_AT_SELECT
		result = select(max_fd+1, &read_fdset, NULL, NULL, 0);
	#else
		result = select(max_fd+1, &read_fdset, NULL, NULL, &lease_time);
	#endif
		if ( result > 0 ) {
			/* 2021/03/24, Jared */
			/* It's not necessary to check the client socket fd in ESXi 5.x but it would got segmentation fail in ESXi 6.x. So I add this check. */
			if ( vsock.client_sockfd != -1 ) {
				if (FD_ISSET(vsock.client_sockfd, &read_fdset)) {
					int bytes = 0;

					memset(&pkt, 0, sizeof(pkt));
					bytes = read(vsock.client_sockfd, &pkt, sizeof(pkt));
#ifdef DEBUG_DUMP_PACKET
					printf("read %d bytes from client:\n", bytes);
					dump_vmci_packet(&pkt);
#endif
					if ( bytes <= 0 ) {
						FD_CLR(vsock.client_sockfd, &read_fdset);
						max_fd = vsock.server_sockfd;
						close(vsock.client_sockfd);
						vsock.client_sockfd = -1; // Set as -1 indicate the client is disconnected.
					}
					else {
						result = parse_and_dispatch(&pkt, &ret_pkt);
						bytes = write(vsock.client_sockfd, &ret_pkt, sizeof(ret_pkt));
						if ( bytes <= 0 ) {
							FD_CLR(vsock.client_sockfd, &read_fdset);
							max_fd = vsock.server_sockfd;
						}
#ifdef _CLOSE_AFTER_WRITE_ /* Jared, close the connection after the packet replied */
						close(vsock.client_sockfd);
						vsock.client_sockfd = -1; // Set as -1 indicate the client is disconnected.
#endif
#ifdef DEBUG_DUMP_PACKET
						printf("write %d bytes to client:\n", bytes);
						dump_vmci_packet(&ret_pkt);
#endif
					}
					/* Semaphore, for accept(), read() mutual exclusion */
					//sem_post(&sem);
				}
			}
			else if (FD_ISSET(vsock.server_sockfd, &read_fdset)) {
				/* Semaphore, for accept(), read() mutual exclusion */
				//sem_wait(&sem);

				client_len = sizeof(struct sockaddr_vm);
				vsock.client_sockfd = accept(vsock.server_sockfd, (struct sockaddr *) &vsock.client_addr, &client_len);
				if ( vsock.client_sockfd == -1 ) {
					perror("accept");
				}
				else {
					FD_SET(vsock.client_sockfd, &read_fdset);
					if ( vsock.client_sockfd > max_fd )
						max_fd = vsock.client_sockfd;
				}
			}
		}
		else if ( result == 0 ) {
#ifdef DEBUG
		//	fprintf(stdout, "timeout to break from select()!!\n");
#endif
		}
		else  {
			fprintf(stdout, "Error occured!!\n");
			/* terminate child process */
			goto main_close;
		}

	} /* End of while(1) */

main_close:
	close(vsock.server_sockfd) ;
	return 0;
}
