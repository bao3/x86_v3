#!/bin/sh
set -x

make_fakeroot()
{
	rm -rf moxa_pled_fakeroot/usr \
	moxa_pled_fakeroot/sig.pkcs7 \
	moxa_pled_fakeroot/DA820PLED
	
	mkdir -p moxa_pled_fakeroot/usr/lib/vmware/vmkmod

	cp -a BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/moxa_pled moxa_pled_fakeroot/usr/lib/vmware/vmkmod/
	chmod +t  moxa_pled_fakeroot/etc/init.d/da820pled

	(cd moxa_pled_fakeroot; \
	tar -czf DA820PLED.tgz usr etc; \
	mv DA820PLED.tgz DA820PLED; \
	touch sig.pkcs7)
}

# Build the vib package
make_vib()
{
	# Collect all the files in moxa_pled_fakeroot/
	make_fakeroot

	# Make the vib file
	# Jared: I can build VIB for VMware ESXi 5.x but it fail for ESXi 6.x.
	# Use vibauthor in CentOS 6.x 32-bits environment can create the right vib file.
	# https://blog.csdn.net/changqing1234/article/details/105800308
	# Please copy the moxa_pled_fakeroot to CentOS 6 32-bits and use the vibauthor to create the vib file.
	#(cd moxa_pled_fakeroot; \
	#ar crv ../DA820PLED.vib descriptor.xml sig.pkcs7 DA820PLED)
}

# Use gcc version 4.1.2-9
# Below is the internal VMWare location.  Please change as required for your
# installed location.
#CC=/build/toolchain/lin32/gcc-4.1.2-9/bin/x86_64-linux-gcc
CC=/build/toolchain/lin64/gcc-4.8.0/bin/x86_64-linux-gcc

# Use ld from binutils-2.17.50.0.15-modcall
# Below is the internal VMWare location.  Please change as required for your
# installed location.
#LD=/build/toolchain/lin32/binutils-2.17.50.0.15-modcall/bin/x86_64-linux-ld
LD=/build/toolchain/lin64/binutils-2.22/bin/x86_64-linux-ld

SYS_ROOT=/nowhere
I_SYSTEM=/build/toolchain/lin64/gcc-4.8.0/lib/gcc/x86_64-linux/4.8.0/include

# Use GNU grep 2.5.1
GREP=grep
# Use GNU sed 4.5.1
SED=sed
# Use GNU xargs 4.2.27
XARGS=xargs
# Use mkdir from GNU coreutils 5.97
MKDIR=mkdir

CFLAGS="--sysroot=$SYS_ROOT \
-fwrapv -pipe -fno-strict-aliasing -Wno-unused-but-set-variable -fno-working-directory -g -ggdb3 -O2 -mcmodel=smallhigh \
-fno-strict-aliasing -freg-struct-return -falign-jumps=1 -falign-functions=4 -falign-loops=1 \
-m64 -mno-red-zone -mpreferred-stack-boundary=4 -minline-all-stringops -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mcld \
-finline-limit=2000 -fno-common -ffreestanding -nostdinc -fomit-frame-pointer \
-nostdlib \
-Wall -Werror -Wstrict-prototypes \
-Wdeclaration-after-statement -Wno-pointer-sign \
-Wno-strict-prototypes -Wno-enum-compare -Wno-switch \
-Wno-declaration-after-statement \
-DCPU=x86-64 \
-DCONFIG_SMP \
-DGPLED_CODE \
-DKBUILD_MODNAME=\"moxa_pled\" \
-DMODULE \
-DLINUX_MODULE_AUX_HEAP_NAME=vmklnx_moxa_pled \
-DLINUX_MODULE_HEAP_INITIAL=1024*100 \
-DLINUX_MODULE_HEAP_MAX=1024*4096 \
-DLINUX_MODULE_HEAP_NAME=vmklnx_moxa_pled \
-DNO_FLOATING_POINT \
-DPRODUCT_VERSION=\"6.7.0\" \
-DVMKERNEL \
-DVMKERNEL_MODULE \
-DVMK_DEVKIT_HAS_API_VMKAPI_BASE \
-DVMK_DEVKIT_HAS_API_VMKAPI_DEVICE \
-DVMK_DEVKIT_HAS_API_VMKAPI_ISCSI \
-DVMK_DEVKIT_HAS_API_VMKAPI_MPP \
-DVMK_DEVKIT_HAS_API_VMKAPI_NET \
-DVMK_DEVKIT_HAS_API_VMKAPI_NPIV \
-DVMK_DEVKIT_HAS_API_VMKAPI_RDMA \
-DVMK_DEVKIT_HAS_API_VMKAPI_SCSI \
-DVMK_DEVKIT_HAS_API_VMKAPI_SOCKETS \
-DVMK_DEVKIT_IS_DDK \
-DVMK_DEVKIT_USES_BINARY_COMPATIBLE_APIS \
-DVMK_DEVKIT_USES_PUBLIC_APIS \
-D__VMKLNX__ \
-D__VMK_GCC_BUG_ALIGNMENT_PADDING__  \
-DVMNIX \
-DVMX86_RELEASE \
-DVMX86_SERVER \
-D_LINUX \
-D_VMKDRVEI -D__KERNEL__ \
-D__VMKERNEL_MODULE__ \
-D__VMKERNEL__ \
-D__VMWARE__ \
-isystem $I_SYSTEM \
-IBLD/build/HEADERS/vmkdrivers-vmkernel/vmkernel64/release \
-IBLD/build/version -IBLD/build/HEADERS/vmkapi-v2_3_0_0-all-public/generic/release \
-IBLD/build/HEADERS/vmkapi-v2_3_0_0-all-public-bincomp/generic/release \
-IBLD/build/HEADERS/vmkapi-current-all-public-bincomp/generic/release \
-IBLD/build/version \
-Ivmkdrivers/vmkplexer/include \
-Ivmkdrivers/vmkplexer/src \
-Ivmkdrivers/vmkplexer/src/base \
-Ivmkdrivers/src_92/include \
-Ivmkdrivers/src_92/include/vmklinux_92 \
-Ivmkdrivers/src_92/vmklinux_92/vmware \
-Ivmkdrivers/src_92/include/linux \
-IBLD/build/HEADERS/92-vmkdrivers-asm-x64/vmkernel64/release \
-IBLD/build/HEADERS/vmkapi-current-all-public-bincomp/generic/release \
-IBLD/build/HEADERS/0-vmkdrivers-namespace/vmkernel64/release/vmkplexer \
-include vmkdrivers/src_92/include/linux/autoconf.h \
"

#-DCONFIG_SMP \
#-DVMK_DEVKIT_USES_BINARY_INCOMPATIBLE_APIS \
#-Ivmkdrivers/src_92/include/vmklinux_9 \

# Create output directories
$GREP -v -e "SED" build-moxa_pled.sh \
| $GREP -o -e "-o [^ ]*\."            \
| $SED -e 's?-o \(.*\)/[^/]*\.?\1?'   \
| $GREP -v -e "\*"                    \
| $XARGS $MKDIR -p

$CC $CFLAGS -c -o BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/SUBDIRS/vmkdrivers/src_9/drivers/char/moxa_pled/moxa_pled.o vmkdrivers/src_9/drivers/char/moxa_pled/moxa_pled.c
$CC $CFLAGS -c -o BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/SUBDIRS/vmkdrivers/src_92/common/vmklinux_module.o vmkdrivers/src_92/common/vmklinux_module.c
$LD -r -o BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/moxa_pled \
--whole-archive \
BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/SUBDIRS/vmkdrivers/src_9/drivers/char/moxa_pled/moxa_pled.o \
BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/SUBDIRS/vmkdrivers/src_92/common/vmklinux_module.o

make_vib
