#ifndef __VMCI_PROTOCOL__
#define __VMCI_PROTOCOL__

#include "pled_ctl.h"
#include "relay_ctl.h"
#include "di_ctl.h"
#include "do_ctl.h"
#include "uartmode_ctl.h"
#include "watchdog_ctl.h"

/* 
 *  according to VMCI socket programming,
 *  pg.11 ... EXSi host always has CID = 2
 */
#define VMSERVER_CID    2

/* Define the return value of each function
 * 0  : success
 * < 0: The error number are defined in bellow
 */
#define E_REQ_NOT_SUPPORT   2
#define E_DEVTYPE_NOT_SUPPORT   3
#define E_INDEX_NOT_AVAILABLE   4
#define E_INDEX_INCORRECT   5
#define E_DEVICE_NOT_OPENED 6
#define E_VALUE_INVALID     7
#define E_SOCKET_INIT_FAIL  11
#define E_SOCKET_CONNECT_FAIL   12

/* MOXA's proprietary communication port number */
#define VMPORT  8888

#define MAX_CONNECTIONS 5

#define DEVICE_LIST_REQUEST	"device_list"
#define DEVICE_OPEN_REQUEST	"device_open"
#define DEVICE_CLOSE_REQUEST	"device_close"
#define DEVICE_GET_REQUEST	"device_get"
#define DEVICE_SET_REQUEST	"device_set"
#define DEVICE_WRITE_REQUEST	"device_write"
#define DEVICE_READ_REQUEST	"device_read"

typedef struct __device_control__ {
    char device_request[32];
    char device_type[32];
    char arg1[32];  /* Argument for request and return value */
    char arg2[32];  /* Reserved */
} device_control;

typedef struct __vmci_socket__ {
    int client_sockfd;
    int server_sockfd;
    int vmciFd; /* VMCI socket programming variables */
    int afVMCI;
    struct sockaddr_vm client_addr;
    struct sockaddr_vm server_addr;
} vmci_socket;

int device_list (int fd);
int device_open (char* type);
void device_close (int fd);
int device_get (int fd, int index, int *value);
int device_set (int fd, int index, int value);
int device_read (int fd, char* data, size_t size);
int device_write (int fd, char* data, size_t size);

#endif
