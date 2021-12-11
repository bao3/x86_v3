#ifndef __PLED_CTRL__
#define __PLED_CTRL__

#define MAX_PLED    8

void turn_on_nth_ledbitmap(int n, char bitmap[]);
void turn_off_nth_ledbitmap(int n, char bitmap[]);
int get_ledbitmap_nth_value(int n, char bitmap[]);

int pled_get(int index, int *value);
int pled_set(int index, int value);
int pled_read(int size, char *data);
int pled_write(int size, char *data);

#endif
