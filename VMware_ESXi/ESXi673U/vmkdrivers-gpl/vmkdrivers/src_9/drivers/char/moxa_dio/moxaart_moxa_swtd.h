#ifndef __MOXAART_MOXA_SWDT__
#define __MOXAART_MOXA_SWDT__

#define DEFAULT_WATCHDOG_TIME	(30UL*1000UL)	// 30 seconds
#define WATCHDOG_MIN_TIME	50UL		// 50 msec
#define WATCHDOG_MAX_TIME	(60UL*1000UL)	// 60 seconds
#define WATCHDOG_TOL_TIME	(50UL)	// 50 msec, for watchdog timer polling
#define WATCHDOG_DEFER_TIME	(15000UL)	// 5 sec, for hw watchdog timer rebooting

#define WATCHDOG_COUNTER(x)	((APB_CLK/1000UL)*(x))
#define WATCHDOG_ACK_JIFFIES(x)	(((x)*HZ/1000UL)-WATCHDOG_TOL_TIME)

#define MOXA_WATCHDOG_MINOR	106
#define IOCTL_WATCHDOG_ENABLE		1	// enable watch dog and set time (unint msec)
#define IOCTL_WATCHDOG_DISABLE		2	// disable watch dog, kernle do it
#define IOCTL_WATCHDOG_GET_SETTING	3	// get now setting about mode and time
#define IOCTL_WATCHDOG_ACK		4	// to ack watch dog

struct swtd_set_struct {
	int		mode;
	unsigned long	time;
};

#endif	//___MOXAART_MOXA_SWDT__
