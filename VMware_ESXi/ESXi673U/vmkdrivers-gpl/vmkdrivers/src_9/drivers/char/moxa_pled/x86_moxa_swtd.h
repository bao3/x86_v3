#ifndef __X86__MOXAWDT__
#define __X86__MOXAWDT__

#define	SUPERIO_CONFIG_PORT		0x2e

#define DEFAULT_WATCHDOG_TIME	(30UL*1000UL)	// 30 seconds
#define WATCHDOG_MIN_TIME	(1UL*1000UL)	//  2 seconds
#define WATCHDOG_MAX_TIME	(255UL*1000UL)	//255 seconds
#define WATCHDOG_TOL_TIME	(50UL)	// 50 msec, for watchdog timer polling
#define WATCHDOG_DEFER_TIME	(15000UL)	// 5 sec, for hw watchdog timer rebooting
#define WATCHDOG_ACK_JIFFIES(x)	(((x)*HZ/1000UL)-WATCHDOG_TOL_TIME)

#define MOXA_WATCHDOG_MINOR	230
#define IOCTL_WATCHDOG_ENABLE		1	// enable watch dog and set time (unint msec)
#define IOCTL_WATCHDOG_DISABLE		2	// disable watch dog, kernle do it
#define IOCTL_WATCHDOG_GET_SETTING	3	// get now setting about mode and time
#define IOCTL_WATCHDOG_ACK		4	// to ack watch dog

#define PLED_PORT	0x306

struct swtd_set_struct {
	int		mode;
	unsigned long	time;
};

unsigned char superio_get_reg(u8 val) {
	outb (val, SUPERIO_CONFIG_PORT);
	//jackie use "asm out 0xeb, al" as delay, it's the same way
	outb( 0x80,0xeb); // a Small delay
	val = inb (SUPERIO_CONFIG_PORT+1);
	outb( 0x80,0xeb); // a Small delay
	return val;
}

void superio_set_reg(u8 val,u8 index) {
	outb (index, SUPERIO_CONFIG_PORT);
	outb( 0x80, 0xeb); // a Small delay
	outb (val, (SUPERIO_CONFIG_PORT+1));
	outb(0x80, 0xeb); // a Small delay
}

void superio_set_logic_device(u8 val) {
	superio_set_reg(val, 0x07);
	outb( 0x80, 0xeb); // a Small delay
}

void superio_enter_config(void) {
#ifndef __WDT_TEST__
	outb (0x87, SUPERIO_CONFIG_PORT);
	outb(0x80, 0xeb); // a Small delay
	outb (0x87, SUPERIO_CONFIG_PORT);
	outb(0x80, 0xeb); // a Small delay
#endif
}

void superio_exit_config(void) {
#ifndef __WDT_TEST__
	outb (0xaa, SUPERIO_CONFIG_PORT);
	outb( 0x80, 0xeb); // a Small delay
#endif
}

#endif	//__X86__MOXAWDT__
