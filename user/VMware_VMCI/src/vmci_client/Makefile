CC=gcc
STRIP=strip
# Jared: We have copy the vmci_sockets.h from open-vm-tools package. So you don't need to install the open-vm-tools and include the headers in open-vm-tools.
#CFLAGS+=-I/usr/src/open-vm-tools-2012.05.21/vsock/shared/
#CFLAGS+=-DDEBUG		# Define for debugging
#CFLAGS+=-DDEBUG_DUMP_PACKET	# Define for dumpping packet

all: mx_exsi_pled mx_exsi_relay

mx_exsi_pled: mx_exsi_pled.o
	$(AR) crv libpled.a 
	$(CC) mx_exsi_pled.o -o $@ $(LDFLAGS)
	$(STRIP) -s $@
	chmod a+x $@
	

mx_exsi_di: mx_exsi_di.o
	$(CC) mx_exsi_di.o -o $@  $(LDFLAGS)
	$(STRIP) -s $@
	chmod a+x $@

mx_exsi_do: mx_exsi_do.o
	$(CC) mx_exsi_do.o -o $@  $(LDFLAGS)
	$(STRIP) -s $@
	chmod a+x $@

mx_exsi_relay: mx_exsi_relay.o
	$(CC) mx_exsi_relay.o -o $@  $(LDFLAGS)
	$(STRIP) -s $@
	chmod a+x $@

%.o:%.c
	$(CC) $(CFLAGS) -c $?

clean:
	rm -rf mx_exsi_pled \
	mx_exsi_relay \
	mx_exsi_di \
	mx_exsi_do \
	*.o \
	*.a

