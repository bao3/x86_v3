#ifndef __UARTMODE__
#define __UARTMODE__

int uartmode_get(int index, int *value);
int uartmode_set(int index, int value);
int uartmode_read(int size, char *data);
int uartmode_write(int size, char *data);

#endif
