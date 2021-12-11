#CC=gcc
CC=/build/toolchain/lin64/gcc-4.8.0/bin/x86_64-linux-gcc
AR=/build/toolchain/lin64/binutils-2.22/bin/x86_64-linux-ar
STRIP=/build/toolchain/lin64/binutils-2.22/bin/x86_64-linux-strip
# Jared: We have copy the vmci_sockets.h from open-vm-tools package. So you don't need to install the open-vm-tools and include the headers in open-vm-tools.
#CFLAGS=-I/usr/src/open-vm-tools-2012.05.21/vsock/shared/ 
CFLAGS+=-I$(shell pwd)/src/include/
#CFLAGS+=-DDEBUG		# Define for debugging
#CFLAGS+=-DDEBUG_DUMP_PACKET	# Define for dumpping packet
#CFLAGS+=-D_CLOSE_AFTER_WRITE_

LDFLAGS=-L$(shell pwd)/src/lib -ldevicectl

export CC AR STRIP CFLAGS LDFLAGS

.PHONY: lib vmci_client vmci_server release_src

all: lib vmci_client vmci_server package

lib:
	$(MAKE) -C src/lib

vmci_server:
	$(MAKE) -C src/vmci_server
	#scp src/vmci_server/mx_vmci_server 10.123.11.18:/tmp/

vmci_client: 
	$(MAKE) -C src/vmci_client
	#scp src/vmci_client/mx_exsi_relay 10.123.11.18:/tmp/

%.o:%.c
	$(CC) $(CFLAGS) -c $?

clean:
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/vmci_server clean
	$(MAKE) -C src/vmci_client clean
	rm -rf *.vib \
	package_vmci_server/usr \
	vmci_example* \
	package_vmci_server/DA820VMCI

.PHONY: package

package: DA820VMCI.vib


release_src:
	mkdir -p vmci_example
	cp -a src win32 vmci_example/
	# Remove the non-released files
	rm -rf vmci_example/src/include/watchdog_ctrl.h \
	vmci_example/src/vmci_client/mx_exsi_watchdog.c \
	vmci_example/src/vmci_server
	# Copy the released version Makefile
	sed -i 's/#include \"watchdog_ctrl.h\"/\/\/#include \"watchdog_ctrl.h\"/g' vmci_example/src/include/device_control.h
	cp -a Makefile.client vmci_example/Makefile
	cp -a README vmci_example/
	tar czvf vmci_example.tar.gz vmci_example
	rm -rf vmci_example


DA820VMCI.vib:
	echo "Create the VMWare package, DA820VMCI.vib"
	echo "Please upload the DA820VMCI.vib to ESXi 6.x host"
	echo "Install command: esxcli software vib install -v /PATH/TO/DA820VMCI.vib"
	rm -rf DA820VMCI.vib sig.pkcs7
	mkdir -p package_vmci_server/usr/lib/vmware/misc/bin
	cp -a src/vmci_server/mx_vmci_server package_vmci_server/usr/lib/vmware/misc/bin/
	chmod +t package_vmci_server/etc/init.d/da820_vmci_server
	(cd package_vmci_server;\
	tar czf DA820VMCI.tgz usr etc;\
	mv DA820VMCI.tgz DA820VMCI;\
	touch sig.pkcs7)
	(cd package_vmci_server;\
	ar crv ../DA820VMCI.vib descriptor.xml sig.pkcs7 DA820VMCI)

