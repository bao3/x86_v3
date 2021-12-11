// Linux Device Driver Template/Skeleton
// Kernel Module

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/* mknod /dev/pled c 10 105 for this module */
#define MOXA_PLED_MINOR 105
#define NAME "pled"
#define LED_NUM 8

/* Debug message */
// #define DEBUG
#ifdef DEBUG
#define dbg_printf(x...)        printk(x)
#else
#define dbg_printf(x...)
#endif

/* Hareware-specific data */
#include "x86_moxa_swtd.h"

/* Implement moxainb() and moxaoutb() 
 * resolving the difference between super io and general outb/inb function 
 */
unsigned char moxainb(unsigned port) {
#if defined DA710
    return superio_get_reg(port);
#elif defined V2100
    return inb(port);
#else
    return inb(port);
#endif 
}

void moxaoutb(unsigned char byte, unsigned port) {
#if defined DA710
    superio_set_reg(byte, port);
#elif defined V2100
    outb(byte, port);
#else
    outb(byte, port);
#endif 
}

// open function - called when the "file" /dev/modelname_io is opened in userspace
static int pled_open (struct inode *inode, struct file *file) {
	dbg_printf("pled_open\n");
	return 0;
}


// close function - called when the "file" /dev/modelname_pled is closed in userspace  
static int pled_release (struct inode *inode, struct file *file) {
	dbg_printf("pled_release\n");
	return 0;
}


/* Write function
 * Note: use echo 11111111 > /dev/pled 
 * The order is [pled1, pled2, pled3, pled4]
 */
ssize_t pled_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	int ret;
	int i;
	unsigned char pled_status;
	char stack_buf[LED_NUM+1];

	dbg_printf("Enter pled_write\n");

	/* check input value */
	dbg_printf("Check the input size\n");
	if ( count != LED_NUM+1 ) { /* 9 = 8 digits + newline */
		printk( KERN_ERR "Moxa pled error! paramter should be 8 digits, like \"11111111\" \n");
		return count;
	}

 	/* parse strint to hex: 
	 * stack_buf=[ pled1 pled2 pled3 pled4 pled5 pled6 pled7 pled8 \n]
	 */
	ret = copy_from_user(stack_buf, buf, count);
	pled_status=0x00;
	for (i = 0; i < LED_NUM; i++) {
	if (stack_buf[i] == '1') 
		/* change status to 0, because low activate */
		pled_status |= 0<<i;
	else if (stack_buf[i] == '0') {
        	pled_status |= 1<<i;
		}
		else {
			printk("pled: error, you input is %s", stack_buf); 
			goto end;
		}
	}
	dbg_printf("val=%x",pled_status);

	/* change status */
	moxaoutb(pled_status, PLED_PORT);
	dbg_printf("pled: set status finish!");
end:
	return count;
}

ssize_t pled_read (struct file *filp, char __user *buf, size_t count,
		loff_t *pos)
{
	int i,  ret = LED_NUM;
	unsigned char pled_status;
	char stack_buf[LED_NUM];
	unsigned char bit_mask = 0x01;

printk("%s[%d]\n", __FUNCTION__, __LINE__);
	if ( count <= LED_NUM ) { /* 8 digits  */
		printk( KERN_ERR "Moxa pled error! paramter should be %d digits.\n", LED_NUM);
		return -EINVAL;
	}

	pled_status=moxainb(PLED_PORT);
	dbg_printf("%s[%d]pled status:%x\n",__FUNCTION__,__LINE__, pled_status);

	memset(stack_buf, '0', LED_NUM *sizeof(char) );
	for (i = 0; i < LED_NUM; i++) {
		if ( !(pled_status & (bit_mask<<i)) )
			stack_buf[i] = '1';
	}
	dbg_printf("%s[%d]pled status:%s\n",__FUNCTION__,__LINE__, stack_buf);

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(char)*LED_NUM);
	if( ret	< 0 ) {
		printk( KERN_ERR "Moxa pled error! paramter should be %d digits, like \n", LED_NUM);
		return -ENOMEM;
	}

	return LED_NUM;
}

// define which file operations are supported
struct file_operations pled_fops = {
	.owner	=	THIS_MODULE,
	.read		= pled_read,/*io_read,*/
	.write	=	pled_write,/*io_write,*/
	.open		= pled_open,
	.release	= pled_release,
	.lock		= NULL,
};

// register as misc driver
static struct miscdevice pled_miscdev = {
	.minor = MOXA_PLED_MINOR,
	.name = "pled",
	.fops = &pled_fops,
};

// initialize module (and interrupt)
static int __init pled_init_module (void) {
	printk("initializing MOXA pled module\n");

	// register misc driver
	if ( misc_register(&pled_miscdev)!=0 ) {
		printk("Moxa pled driver: Register misc fail !\n");
		return -ENOMEM;
	}
#if 0
	moxaoutb(0xff, PLED_PORT);    
#endif
	return 0;
}

// close and cleanup module
static void __exit pled_cleanup_module (void) {
	printk("cleaning up module\n");
	misc_deregister(&pled_miscdev);
}

module_init(pled_init_module);
module_exit(pled_cleanup_module);
MODULE_AUTHOR("wade_liang@moxa.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MOXA pled module");

