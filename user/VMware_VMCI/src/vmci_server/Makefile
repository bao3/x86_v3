
LDFLAGS+=-lrt -lpthread

all: mx_vmci_server

mx_vmci_server: mx_vmci_server.o pled_ctrl.o relay_ctrl.o di_ctrl.o do_ctrl.o #uartmode_ctrl.o
	#$(CC) mx_vmci_server.o pled_ctrl.o di_ctrl.o do_ctrl.o uartmode_ctrl.o -o $@ $(LDFLAGS)
	$(CC) mx_vmci_server.o pled_ctrl.o relay_ctrl.o -o $@ $(LDFLAGS)
	$(STRIP) -s $@
	chmod a+x $@

%.o:%.c
	$(CC) $(CFLAGS) -c $?

clean:
	rm -rf mx_vmci_server \
	*.o *.gdb

