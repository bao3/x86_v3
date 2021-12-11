#ifndef __RELAY__
#define __RELAY__

int relay_get(int index, int *value);
int relay_set(int index, int value);
int relay_read(int size, char *data);
int relay_write(int size, char *data);

#endif
