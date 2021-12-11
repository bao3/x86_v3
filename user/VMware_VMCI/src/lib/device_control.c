#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <WS2tcpip.h>
#endif // end of WIN32
#include "vmci_sockets.h"
#include "device_control.h"	/* Define the packaet format */

#ifdef WIN32
#pragma comment (lib, "Ws2_32.lib")
#endif // end of WIN32

/* total size of protocol between host & guest */
#define MAX_CMD_SIZE	256

#ifdef WIN32
void close (
	_In_ SOCKET s
	)
{
    //Close the socket if it exists
    if (s)
        closesocket(s);

    WSACleanup(); //Clean up Winsock
}

int write(
	_In_  SOCKET s,
	_In_  const char *buf,
	_In_  int len
)
{
	return send ( s, buf, len, 0);
}

int read(
	_In_   SOCKET s,
	_Out_  char *buf,
	_In_   int len
)
{
	return recv( s, buf, len, 0);
}

#endif // end of WIN32

/* Initialize the client socket */
int init_client_socket(vmci_socket *pSock) {

	unsigned long long setBuf = MAX_CMD_SIZE;
	socklen_t setBufSize=sizeof(setBuf);
	unsigned long long getBuf;
	socklen_t getBufSize=sizeof(getBuf);

#ifdef WIN32
	WSADATA wsaData;
	int iResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        goto init_client_socket_exit;
    }
#endif

	pSock->afVMCI = VMCISock_GetAFValueFd(&pSock->vmciFd);

	memset((char*)&pSock->client_addr, 0, sizeof(pSock->client_addr));
	pSock->client_addr.svm_family = pSock->afVMCI;
	pSock->client_addr.svm_cid = VMSERVER_CID;
	pSock->client_addr.svm_port = VMPORT;

	if ( (pSock->client_sockfd=socket(pSock->afVMCI, SOCK_STREAM , 0)) == -1 ) {
		perror("Create client socket fail");
		exit(0);
	}
#ifdef DEBUG
	fprintf(stdout, "VMCISock_GetLocalCID is %d\n", VMCISock_GetLocalCID());
#endif
	/* reduce buffer to above size and check */
	if (setsockopt(pSock->client_sockfd, pSock->afVMCI, SO_VMCI_BUFFER_SIZE, (void *)&setBuf, sizeof setBuf) == -1) 
	{
		perror("setsockopt");
		goto init_client_socket_exit;
	}

	if (getsockopt(pSock->client_sockfd, pSock->afVMCI, SO_VMCI_BUFFER_SIZE, (void *)&getBuf, &getBufSize) == -1)
	{
		perror("getsockopt");
		goto init_client_socket_exit;
	}

	/* compare what we had set */
	if (getBuf != setBuf) {
		fprintf(stderr, "SO_VMCI_BUFFER_SIZE not set to size requested.\n");
		goto init_client_socket_exit;
	}

	return 0;

init_client_socket_exit:
	close(pSock->client_sockfd);
	return -E_SOCKET_INIT_FAIL;
}

#ifndef WIN32
/* Initialize the server socket */
int init_server_socket(vmci_socket *pSock) {
	unsigned long long setBuf = 32768;
	unsigned long long setBufSize=sizeof(setBuf);
	unsigned long long getBuf;
	socklen_t getBufSize=sizeof(getBuf);

	// Use -1 as flag to identify the socket has not accepted.
	pSock->client_sockfd = -1; 
	pSock->server_sockfd = -1;

	memset((char*)&pSock->client_addr, 0, sizeof(struct sockaddr_vm));
	memset((char*)&pSock->server_addr, 0, sizeof(struct sockaddr_vm));
#ifdef DEBUG
	fprintf(stdout, "VMCISock_GetLocalCID is %d\n", VMCISock_GetLocalCID());
#endif

	pSock->afVMCI = VMCISock_GetAFValueFd(&pSock->vmciFd);
	if ((pSock->server_sockfd=socket(pSock->afVMCI, SOCK_STREAM, 0)) == -1) {
		perror("Create server socket fail");
		exit(0);
       	}

	pSock->server_addr.svm_family = pSock->afVMCI;
	pSock->server_addr.svm_cid = VMADDR_CID_ANY;
	//server_addr.svm_port = VMADDR_PORT_ANY;
	pSock->server_addr.svm_port = VMPORT;

	/* reduce buffer to above size and check */
	if (setsockopt(pSock->server_sockfd, pSock->afVMCI, SO_VMCI_BUFFER_SIZE, (void *)&setBuf, sizeof(setBuf)) == -1) {
		perror("setsockopt");
		goto init_server_socket_exit;
	}

	if (getsockopt(pSock->server_sockfd, pSock->afVMCI, SO_VMCI_BUFFER_SIZE, (void *)&getBuf, &getBufSize) == -1) {
		perror("getsockopt");
		goto init_server_socket_exit;
	}

	if (fcntl(pSock->server_sockfd, F_SETFL, fcntl(pSock->server_sockfd, F_GETFL) | O_NONBLOCK) < 0) {
		perror("fcntl O_NONBLOCK");
		goto init_server_socket_exit;
	}

	if (getBuf != setBuf) {
		fprintf(stderr, "SO_VMCI_BUFFER_SIZE not set to size requested.\n");
		goto init_server_socket_exit;
	}

	if (bind(pSock->server_sockfd, (struct sockaddr *) &pSock->server_addr, sizeof(pSock->server_addr)) == -1) {
		perror("bind");
		goto init_server_socket_exit;
	}

	/* Only accept 1 connect once */
	if (listen(pSock->server_sockfd, MAX_CONNECTIONS) == -1) {
		perror("listen");
		goto init_server_socket_exit;
	}

	return 0;

init_server_socket_exit:
	close(pSock->server_sockfd);
	return -E_SOCKET_INIT_FAIL;
}
#endif // end of WIN32

int connect_client_socket(vmci_socket *pSock) {

	int result ;
	
	result = connect(pSock->client_sockfd, (struct sockaddr *)&pSock->client_addr, sizeof(pSock->client_addr)) ;
	if(result == -1) {
		perror("connect_client_socket(): connection failure:") ;
		goto connect_client_socket_exit;
	}

	return 0;

connect_client_socket_exit:
	return  -E_SOCKET_CONNECT_FAIL;
}

void dump_vmci_packet(device_control *pPkt) {

	if ( pPkt->device_request )
		printf(	"device_request:%s\n", pPkt->device_request);
	if ( pPkt->device_type )
		printf("device_type:%s\n", pPkt->device_type);
	if ( pPkt->arg1 )
		printf("arg1:%s\n", pPkt->arg1);
	if ( pPkt->arg2 )
		printf("arg2:%s\n", pPkt->arg2);
}

int device_list (int fd) {
	vmci_socket vsock_list;
	int result = 0;
	device_control request_packet= {
		DEVICE_LIST_REQUEST,
		0,
		0,
		0,
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_list_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_list_exit;

	/* 2. Request device_list with device_type */
	sprintf(request_packet.device_type, "%d", fd);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_list_exit;
	}

	//usleep(50);

	/* 3. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_list_exit;
	}

#ifdef DEBUG_DUMP_PACKET
	printf("Receive from server:\n");
	dump_vmci_packet(&request_packet);
#endif

	/* 4. Close the connection */
	close(vsock_list.client_sockfd) ;

	/* 5. Return a value for success or fail */
	return atoi(request_packet.arg1);

device_list_exit:
	close(vsock_list.client_sockfd) ;
	return result;
}

int device_open (char* type) {
	vmci_socket vsock_list;
	int result = 0;
	device_control request_packet= {
		DEVICE_OPEN_REQUEST,
		0,
		0,
		0,
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_open_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_open_exit;

	/* 2. Request device_open with device_type */
	strcpy(request_packet.device_type, type);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_open_exit;
	}

	//usleep(50);

	/* 3. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_open_exit;
	}

#ifdef DEBUG_DUMP_PACKET
	printf("Receive %d from server:\n", result);
	dump_vmci_packet(&request_packet);
#endif

	/* 4. Close the connection */
	close(vsock_list.client_sockfd) ;

	/* 5. Return a value for success or fail */
	return atoi(request_packet.arg1);

device_open_exit:
	close(vsock_list.client_sockfd) ;
	return result;
}

void device_close (int fd) {
	vmci_socket vsock_list;
	int result=0;
	device_control request_packet= {
		DEVICE_CLOSE_REQUEST,
		0,
		0,
		0,
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_close_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_close_exit;

	/* 2. Request device_close with fd */
	sprintf(request_packet.device_type, "%d", fd);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_close_exit;
	}

	//usleep(50);

	/* 3. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_close_exit;
	}

#ifdef DEBUG_DUMP_PACKET
	printf("Receive from server:\n");
	dump_vmci_packet(&request_packet);
#endif

device_close_exit:
	close(vsock_list.client_sockfd) ;
	return;
}

int device_get (int fd, int index, int *value) {
	vmci_socket vsock_list;
	int result = 0;
	device_control request_packet= {
		DEVICE_GET_REQUEST,
		0,
		0,
		0,
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_get_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_get_exit;

	/* 2. Request device_close with fd */
	sprintf(request_packet.device_type, "%d", fd);

	/* 3. arg1 = index */
	sprintf(request_packet.arg1, "%d", index);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_get_exit;
	}

	//usleep(50);

	/* 3. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_get_exit;
	}

	*value = atoi(request_packet.arg1);

#ifdef DEBUG_DUMP_PACKET
	printf("Receive from server:\n");
	dump_vmci_packet(&request_packet);
#endif

	/* 4. Close the connection */
	close(vsock_list.client_sockfd) ;

	/* 5. Return a value for success or fail */
	return atoi(request_packet.arg1);

device_get_exit:
	close(vsock_list.client_sockfd) ;
	return result;
}

int device_set (int fd, int index, int value) {
	vmci_socket vsock_list;
	int result = 0;
	device_control request_packet= {
		DEVICE_SET_REQUEST,
		0,
		0,
		0,
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_set_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_set_exit;

	/* 2. Request device_close with fd */
	sprintf(request_packet.device_type, "%d", fd);

	/* 3. arg1 = index */
	sprintf(request_packet.arg1, "%d", index);

	/* 4. arg2 = value */
	sprintf(request_packet.arg2, "%d", value);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_set_exit;
	}

	//usleep(50);

	/* 5. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_set_exit;
	}

#ifdef DEBUG_DUMP_PACKET
	printf("Receive from server:\n");
	dump_vmci_packet(&request_packet);
#endif

	/* 6. Close the connection */
	close(vsock_list.client_sockfd) ;

	/* 7. Return a value for success or fail */
	return atoi(request_packet.arg1);

device_set_exit:
	close(vsock_list.client_sockfd) ;
	return result;
}

int device_read (int fd, char* data, size_t size) {
	vmci_socket vsock_list;
	int result = 0;
	int len;
	device_control request_packet= {
		DEVICE_READ_REQUEST,
		0,
		0,
		0,
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_read_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_read_exit;

	/* 2. Request device_read with fd */
	sprintf(request_packet.device_type, "%d", fd);
	sprintf(request_packet.arg1, "%d", size);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_read_exit;
	}

	//usleep(50);

	/* 3. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_read_exit;
	}

	/* Get the len of data */
	len = atoi(request_packet.arg1);
	if( len <= 0 ) {
		printf("len:%d is incorrect\n", len);
		goto device_read_exit;
	}

	memcpy(data, request_packet.arg2, size);

#ifdef DEBUG_DUMP_PACKET
	printf("Receive from server:\n");
	dump_vmci_packet(&request_packet);
#endif

	/* 4. Close the connection */
	close(vsock_list.client_sockfd) ;

	/* 5. Return a value for success or fail */
	return len;

device_read_exit:
	close(vsock_list.client_sockfd) ;
	return result;
}

int device_write (int fd, char* data, size_t size) {
	vmci_socket vsock_list;
	int result = 0;
	int len;
	device_control request_packet= {
		DEVICE_WRITE_REQUEST,
		0,
		0,
		0
	};

	/* 1. Connect to the server */
	result = init_client_socket(&vsock_list);
	if( result < 0 )
		goto device_write_exit;

	result = connect_client_socket(&vsock_list);
	if( result < 0 )
		goto device_write_exit;

	/* 2. Request device_read with fd */
	sprintf(request_packet.device_type, "%d", fd);
	sprintf(request_packet.arg1, "%d", size);
	sprintf(request_packet.arg2, "%s", data);

#ifdef DEBUG_DUMP_PACKET
	printf("Write to server:\n");
	dump_vmci_packet(&request_packet);
#endif

	result = write(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("write to client_sockfd fail:");
		goto device_write_exit;
	}

	//usleep(50);

	/* 3. Reply success or fail via arg1 */
	result = read(vsock_list.client_sockfd, (char*)&request_packet, sizeof(request_packet)) ;
	if( result < 0 ) {
		perror("read from client_sockfd fail:");
		goto device_write_exit;
	}

	/* Get the len of data */
	len = atoi(request_packet.arg1);
	if( len <= 0 ) {
		printf("request_packet.arg1:%s, len:%d is incorrect\n", request_packet.arg1, len);
		goto device_write_exit;
	}

	memcpy(data, request_packet.arg2, size);

#ifdef DEBUG_DUMP_PACKET
	printf("Receive from server:\n");
	dump_vmci_packet(&request_packet);
#endif

	/* 4. Close the connection */
	close(vsock_list.client_sockfd) ;

	/* 5. Return a value for success or fail */
	return len;

device_write_exit:
	close(vsock_list.client_sockfd) ;
	return result;
}

