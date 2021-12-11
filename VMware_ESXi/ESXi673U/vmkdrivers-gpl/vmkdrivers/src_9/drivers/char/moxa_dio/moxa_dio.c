// Linux Device Driver Template/Skeleton
// Kernel Module

#include <linux/crc32.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/prefetch.h>
#include <linux/debugfs.h>
#include <asm/irq.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <asm/io.h>
#include "x86_moxa_swtd.h"

/* mknod /dev/dio c 10 104 for this module */
#define MOXA_DIO_MINOR 104
#define NAME "dio"

/*
 * DIO file operaiton function call
 */

#define DIO_INPUT               1
#define DIO_OUTPUT              0
#define DIO_HIGH                1
#define DIO_LOW                 0
#define IOCTL_DIO_GET_MODE      1
#define IOCTL_DIO_SET_MODE      2
#define IOCTL_DIO_GET_DATA      3
#define IOCTL_DIO_SET_DATA      4
#if 0
#define IOCTL_SET_DOUT	_IOW(MX_DI_MINOR,15,int)
#define IOCTL_GET_DOUT	_IOW(MX_DI_MINOR,16,int)
#define IOCTL_GET_DIN	_IOW(MX_DI_MINOR,17,int)
#define IOCTL_GET_RELAY	_IOW(MX_DI_MINOR,18,int) /* Not supported */
#define IOCTL_SET_RELAY	_IOW(MX_DI_MINOR,19,int)
#else
#define IOCTL_SET_DOUT          15
#define IOCTL_GET_DOUT          16
#define IOCTL_GET_DIN           17
#define IOCTL_GET_RELAY         18
#define IOCTL_SET_RELAY         19
#endif


/*
 * DP - debug print
 */
//#define DEBUG
#ifdef DEBUG 
#define DP(fmt, args...) printk("%s,%d: "fmt,__FUNCTION__,__LINE__,##args)
#else
#define DP(fmt, args...) 
#endif

/* 
 * Hareware-specific data:
 * This driver will use the public dio order [DI DO], 
 * if your platdorm doesn't follow this order,
 * we use PUBLIC_ORDER/PRIVATE_ORDER to convert
 */

#define CHANGE_DIO_ORDER(var) (var>>4) | ((var & 0xf) <<4)
#define DI_VAL(var) var >> 4
#define DO_VAL(var) var & 0xf

#define MAX_DO                 3
#define MAX_RELAY              1
#define MAX_DI                 6
//#define BASEPORT            
#define	DIO_IOPORT	/* Define for general IO control */
#define ADDR_DO_PORT 	    0x302
#define ADDR_RELAY_PORT     0x305
#define ADDR_DI_PORT 	    0x307
#define ADDR_SERIAL_OPMODE  0x300

#define RELAY_BIT	5

#ifdef DIO_SUPERIO	/* DA-820C use inb()/outb() IO control */
#undef DIO_SUPERIO
#endif

#define PUBLIC_ORDER(var)       CHANGE_DIO_ORDER(var)
#define PRIVATE_ORDER(var)      CHANGE_DIO_ORDER(var)

#define MOXA	0x400
#define MOXA_SET_OP_MODE	(MOXA + 66)
#define MOXA_GET_OP_MODE	(MOXA + 67)

#define RS232_MODE		0
#define RS485_2WIRE_MODE	1
#define RS422_MODE		2
#define RS485_4WIRE_MODE	3
#define OP_MODE_MASK		3


struct dio_set_struct {
        int     io_number;
        int     mode_data;// 1 for input, 0 for output, 1 for high, 0 for low
};

static unsigned char do_state_keep=0x00; //use use public order: [DI DO]

static unsigned char relay_state_keep=0x00;
unsigned char    keep_opmode=0x00;


/* 
 *  dio_inb() and dio_outb() - called by ioctl and init for merging ioport and superio
 *  here we change order dio order to public order(di,do) for driver operation.
 *  Note: this two function can't use in serial mode operation
 */

static unsigned char dio_inb(unsigned port) {
    unsigned char val;
    #ifdef DIO_SUPERIO
    superio_enter_config();
    superio_set_logic_device(9);//GPIO
    val = superio_get_reg(port);
    superio_exit_config();
    #elif (defined DIO_IOPORT)
    val = inb(port);
    #endif 
    DP("data in private order: %x\n", val);
    return PUBLIC_ORDER(val);
}

static void dio_outb(unsigned char byte, unsigned port) {
    DP("data in public order: %x\n", byte);
    /* Note: you must use = to assign value */
    byte = PRIVATE_ORDER(byte);
    #if defined DIO_SUPERIO
    DP("data in private order: %x\n", byte);
    superio_enter_config();
    superio_set_logic_device(9);//GPIO
    superio_set_reg(byte, port);
    superio_exit_config();
    #elif defined DIO_IOPORT
    outb(byte, port);
    #endif 
}

/* 
 * Open/release
 */

static int io_open (struct inode *inode, struct file *file) {
	DP("io_open\n");
	return 0;
}

static int io_release (struct inode *inode, struct file *file) {
	DP("io_release\n");
	return 0;
}


/* 
 *  Relay Ioctl - I/O control 
 */
#if 0
static long relay_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
	int value;

	switch ( cmd ) {
	case IOCTL_SET_RELAY:
		printk("<1>%s[%d]arg:%lu\n", __FUNCTION__, __LINE__, arg );
		 if ( arg == DIO_HIGH )
			relay_state_keep |= (1<<RELAY_BIT); // 0x305 bit 5, passed from user
	        else if ( arg == DIO_LOW )
	                relay_state_keep &= ~(1<<RELAY_BIT);
	        else
	                return -EINVAL;

		printk("<1>%s[%d]relay_state_keep:%x\n", __FUNCTION__, __LINE__, relay_state_keep );
	        outb(relay_state_keep,ADDR_RELAY_PORT);
	        DP("relay_state_keep:%x\n", (unsigned char)relay_state_keep);
		break;

	case IOCTL_GET_RELAY:

		if ( inb(ADDR_RELAY_PORT) & (1<<RELAY_BIT) ) {
			value  = 1;
		}
		else {
			value = 0;
		}

		DP("mode_data: %x\n", value);

		return value;
		break;

	default:
		printk("realy_ioclt() ioctl invail\n");
		return -EINVAL;
	}

	return 0;

}
#endif

#ifdef ORIGINAL_IOCTL
static int io_ioctl(struct inode *inode, struct file *file,unsigned int cmd, unsigned long arg)
#else
static long io_ioctl(struct file *file, unsigned cmd, unsigned long arg)
#endif
{
	struct dio_set_struct   set;
#if (defined V2100) || (defined DA820C)
	unsigned char port,opmode,val;
#endif

	switch ( cmd ) {
	case IOCTL_SET_RELAY:
		if ( copy_from_user(&set, (struct dio_set_struct *)arg, sizeof(struct dio_set_struct)) )
			return -EFAULT;

		if ( set.mode_data == DIO_HIGH )
			relay_state_keep |= (1<<RELAY_BIT); // 0x305 bit 5, passed from user
	        else if ( set.mode_data == DIO_LOW )
	                relay_state_keep &= ~(1<<RELAY_BIT);
	        else
	                return -EINVAL;

	        outb(relay_state_keep,ADDR_RELAY_PORT);
	        DP("relay_state_keep:%x\n", (unsigned char)relay_state_keep);
		break;

	case IOCTL_GET_RELAY:

		if ( copy_from_user(&set, (struct dio_set_struct *)arg, sizeof(struct dio_set_struct)) )
			return -EFAULT;

		if ( inb(ADDR_RELAY_PORT) & (1<<RELAY_BIT) ) {
			set.mode_data  = DIO_HIGH;
		}
		else {
			set.mode_data = DIO_LOW;
		}

		DP("mode_data: %x\n", set.mode_data);

		if (copy_to_user((struct dio_set_struct *)arg, &set, sizeof(struct dio_set_struct)))
			return -EFAULT;

		break;

	case IOCTL_SET_DOUT :
		DP("set dio\n");
	if ( copy_from_user(&set, (struct dio_set_struct *)arg, sizeof(struct dio_set_struct)) )
		return -EFAULT;
        else if ( set.io_number < 0 || set.io_number >= MAX_DO )
		return -EINVAL;
        if ( set.mode_data == DIO_HIGH )
		do_state_keep |= (1<<set.io_number);
        else if ( set.mode_data == DIO_LOW )
                do_state_keep &= ~(1<<set.io_number);
        else
            return -EINVAL;
        dio_outb(do_state_keep,ADDR_DO_PORT);
        DP("do_state_keep:%x\n", (unsigned char)do_state_keep);	

	break;

    case IOCTL_GET_DOUT :
        DP("get dout\n");

    case IOCTL_GET_DIN :
        DP("get din\n");

        /* code below is for get_din/dout */

        if ( copy_from_user(&set, (struct dio_set_struct *)arg, sizeof(struct dio_set_struct)) )
            return -EFAULT;
        /* case 1: for all port */
        if ( set.io_number == -1 ) {	
             if ( cmd == IOCTL_GET_DOUT ) 
                set.mode_data = DO_VAL(dio_inb(ADDR_DI_PORT));
             else  
                set.mode_data = DI_VAL(dio_inb(ADDR_DI_PORT));
             DP("all port: %x",set.mode_data);

        /* case 2: for single port */
        } else {
            if ( set.io_number < 0 || set.io_number >= MAX_DI )
                return -EINVAL;
            if ( cmd == IOCTL_GET_DOUT ) {
                if ( DO_VAL(dio_inb(ADDR_DI_PORT)) & (1<<set.io_number) )
                    set.mode_data = 1;
                else
                    set.mode_data = 0;
            } else {
                    DP("compare DI_VAL: %x, io_number: %x", DI_VAL(dio_inb(ADDR_DI_PORT)), 1<<set.io_number);
                if ( DI_VAL(dio_inb(ADDR_DI_PORT)) & (1<<set.io_number) ){
                    set.mode_data = 1;
                } else {
                    set.mode_data = 0;
                }
            }
        }

        if (copy_to_user((struct dio_set_struct *)arg, &set, sizeof(struct dio_set_struct)))
		return -EFAULT;
        DP("mode_data: %x\n",(unsigned int)set.mode_data);
		break;

#if (defined V2100) || (defined DA820C)
    /* SERIAL PORT OP MODE */
	case MOXA_SET_OP_MODE:
		copy_from_user(&opmode,(unsigned char *)arg,sizeof(unsigned char));
		port = opmode >> 4 ;
		opmode = opmode & 0xf;

		if ( opmode != RS232_MODE && opmode != RS485_2WIRE_MODE && opmode != RS422_MODE && opmode != RS485_4WIRE_MODE && port > 1)
			return -EFAULT;

		DP("port:%x,opmode:%x\n",port,opmode);
		val=inb(ADDR_SERIAL_OPMODE)&(~(((unsigned char)0xe)<<(4*port)));
		DP("val:%x\n",val);

		switch(opmode){
		case RS232_MODE:
			val|=(((unsigned char)0x8)<<(4*port));
			break;			
		case RS485_2WIRE_MODE:
			val|=(((unsigned char)0x2)<<(4*port));
			break;
		case RS422_MODE:
		case RS485_4WIRE_MODE:
			val|=(((unsigned char)0x4)<<(4*port));
			break;
		}

		DP("val:%x\n",val);
		outb(val,ADDR_SERIAL_OPMODE);

		keep_opmode &= ~(((unsigned char)0xf)<<(port*4));
		keep_opmode |= opmode<<(port*4);
	#if (defined DIO_SUPERIO)
		superio_enter_config();
		superio_set_logic_device((u8)(port+1));
		if(opmode == RS232_MODE){
			val=superio_get_reg(0xf0)& 0x7f;
		}else{
			val=superio_get_reg(0xf0)| 0x80;
		}
		superio_set_reg(val , 0xf0);
	#endif
		break;

	case MOXA_GET_OP_MODE:
		copy_from_user(&port,(unsigned char *)arg,sizeof(unsigned char));
		if(port>1)
			return -EINVAL;
		opmode=(keep_opmode>>(port*4))& 0xf;
		copy_to_user((unsigned char*)arg, &opmode, sizeof(unsigned char));
		break;
#endif
	default:
		printk("ioctl invail\n");
		return -EINVAL;
	}

	return 0;
}

struct file_operations io_fops = {
	.owner	=	THIS_MODULE,
#ifdef ORIGINAL_IOCTL
	.ioctl	=   io_ioctl,
#else
	//.unlocked_ioctl	=   relay_ioctl,
	.unlocked_ioctl	=   io_ioctl,
#endif
	.open		=   io_open,
	.release	=   io_release,
	.lock		=	NULL,
};

static struct miscdevice dio_miscdev = {
    .minor = MOXA_DIO_MINOR,
    .name = "dio",
    .fops = &io_fops,
};

static int __devinit io_init_module (void) {
	printk("initializing MOXA dio module\n");
    int result;

    /* register misc driver */
    if ((result=misc_register(&dio_miscdev)) != 0 ) {
        printk("Moxa dio driver: Register misc fail !\n");
        return result;
    }

    /* initialize for diffrent platform */
    #if (defined DIO_SUPERIO)
	outb(0x00, BASEPORT);//lighten the MOXA led
	outb(0x88, ADDR_SERIAL_OPMODE); //set default serial mode to RS232
	superio_enter_config();
	superio_set_logic_device(1);
	val=superio_get_reg(0xf0)&0x7f;
	superio_set_reg(val,0xf0);
	superio_enter_config();
	superio_set_logic_device(2);
	val=superio_get_reg(0xf0)&0x7f;
	superio_set_reg(val,0xf0);
	DP("ADDR_OP_MODE:%x\n",(unsigned int)inb(ADDR_SERIAL_OPMODE));
    #endif
    
    /* Note: dio_outb may change dio order, but it works well with 
     * dio_state_keep(0x00) at initial state
     */
#if 0	/* Don't set the initial value for DI/DO */
	dio_outb(do_state_keep, ADDR_DO_PORT);
#endif
	return 0;
}

/*
 * close and cleanup module
 */
static void __exit io_cleanup_module (void) {
	printk("cleaning up module\n");
	misc_deregister(&dio_miscdev);
}

module_init(io_init_module);
module_exit(io_cleanup_module);
MODULE_AUTHOR("jared.wu@moxa.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MOXA DIO module for VMWare EXSi");

