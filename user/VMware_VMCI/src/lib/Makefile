
all: libdevicectl.a

libdevicectl.a: device_control.o
	$(AR) crv libdevicectl.a device_control.o 

%.o:%.c
	$(CC) $(CFLAGS) -c $?

clean:
	rm -rf *.a *.o

