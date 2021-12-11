#ifndef __DI__
#define __DI__

int di_get(int index, int *value);
int di_set(int index, int value);
int di_read(int size, char *data);
int di_write(int size, char *data);

#endif
