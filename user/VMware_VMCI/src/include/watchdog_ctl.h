#ifndef __WATCHDOG_CTRL__
#define __WATCHDOG_CTRL__

int watchdog_get(int index, int *value);
int watchdog_set(int index, int value);
int watchdog_read(int size, char *data);
int watchdog_write(int size, char *data);

#endif
