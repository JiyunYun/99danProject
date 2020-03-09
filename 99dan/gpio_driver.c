#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/ioport.h> 
#include <linux/fs.h> 

#include <asm/uaccess.h> 
#include <asm/hardware.h>
#include <asm/irq.h>
#include <linux/irq.h>
#include <asm/arch/pxa-regs.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/timer.h>


/*
 * 함수 Prototypes정의 
 */
int	led_gpio_open(struct inode *, struct file *);
int	led_gpio_release(struct inode *, struct file *);
ssize_t	led_gpio_write(struct file *, const char *, size_t, loff_t *);
static 	irqreturn_t	pushsw_handler(void);
static void ledoff_timeout( unsigned long unused );
static void pushswitch_timeout( unsigned long unused );

/*
 * The name for our device, as it will appear in /proc/devices
 */
#define	DEVICE_NAME	"gpio_test"  	// LED 디바이스 명 
#define LED_MAJOR	257		// LED 디바이스 메이저 번호
#define GPIO83_ON	0x01
#define GPIO82_ON	0x02
#define GPIO83_OFF	0x10
#define GPIO82_OFF	0x20
#define ALL_ON		0x03
#define ALL_OFF		0x30
	
static struct file_operations led_fops =
{
	open:		led_gpio_open,
	write:		led_gpio_write,	
	release:	led_gpio_release,
};

static struct timer_list ledoff_timer, pushswitch_timer;
static int Major;
static int Device_Open = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");

static int __init led_gpio_init(void)
{
 	/*
  	* Register the character device
  	*/
	Major = register_chrdev (LED_MAJOR, DEVICE_NAME, &led_fops);
    
     	/* Negative values signify an error */
	if (Major < 0) {
		printk ("LED init_module: failed with %d\n", Major);
		return Major;
	}  
   	printk ("[%s] registred: major number = %d\n", DEVICE_NAME, LED_MAJOR);
    return 0;
}

static irqreturn_t pushsw_handler(void)
{
	mod_timer( &pushswitch_timer, jiffies + 2); 
	
	return IRQ_HANDLED;
}

static void ledoff_timeout( unsigned long unused )
{
	timer_pending( &ledoff_timer );
	GPSR2 = 1<<18;
	printk( "\tLed[82] off!\n" );
}
	
static void pushswitch_timeout( unsigned long unused )
{
        timer_pending( &pushswitch_timer );
	if( GPLR(32) == 0 ) return;

	GPCR2 = 1<<18;
        printk( "\tPUSH GPIO32 \n" );

	mod_timer( &ledoff_timer, jiffies + 100); 
}

int led_gpio_open (struct inode *inode, struct file *file)
{
	/*
	 * Get major / minor numbers when needed
	 */	
	
 	int return_val;
	if (Device_Open) {
		return -EBUSY;
	}
	Device_Open++;

	/*
	 * GPIO32,82,83 GPIO mode
	 */
	pxa_gpio_mode(32 | GPIO_IN);
	pxa_gpio_mode(82 | GPIO_OUT);
	pxa_gpio_mode(83 | GPIO_OUT);

	/*
	 * initialize led off timeout timer
	 */
	init_timer( &ledoff_timer );
	init_timer( &pushswitch_timer );
    	ledoff_timer.function = ledoff_timeout;
    	pushswitch_timer.function = pushswitch_timeout;

	set_irq_type( IRQ_GPIO(32), IRQ_TYPE_EDGE_RISING );  
	return_val = request_irq(IRQ_GPIO(32), (void *)pushsw_handler, SA_INTERRUPT, "PUSH SW 32 INT", NULL );

    	if( return_val < 0 )
    	{
        	printk(KERN_ERR "xstation_push_init() : Can't reqeust irq %#010x\n", IRQ_GPIO(32) );
        	return -1;
    	}

	return 0;
}

int led_gpio_release (struct inode *inode, struct file *file)
{
	/* We're now ready for our next caller */
	Device_Open--;

	free_irq(IRQ_GPIO(32), NULL);
    	del_timer( &ledoff_timer );
    	del_timer( &pushswitch_timer );

	return 0;
}

ssize_t led_gpio_write (struct file *file,
			     const char *buffer,	/* buffer */
			     size_t length,		/* length of buffer */
			     loff_t * offset)		/* offset in the file */
{	
	const char *tmp = buffer;
	unsigned char c=0;
	
	get_user( c, (unsigned char *)(tmp) );	 	
 	switch(c){
		case GPIO82_ON  : GPCR2 = 1 << 18; break;  // GPIO82 ON
                case GPIO83_ON  : GPCR2 = 1 << 19; break;  // GPIO83 ON
                case GPIO82_OFF : GPSR2 = 1 << 18; break;  // GPIO82 OFF
                case GPIO83_OFF : GPSR2 = 1 << 19; break;  // GPIO38 OFF
                case ALL_ON     : GPCR2 = (1 << 18);
                                  GPCR2 = (1 << 19);
                                  break; 		// GPIO82,83 ON
                case ALL_OFF    : GPSR2 = (1 << 18);
                                  GPSR2 = (1 << 19);
                                  break;		// GPIO82,83 OFF
 		default		: printk("\n LED Write Error "); break;
 	}
	return (length);
}

void __exit led_gpio_exit(void)
{
	int ret;
	/*
	 * Unregister the device
	 */
	ret = unregister_chrdev (LED_MAJOR, DEVICE_NAME); 

	/*
	 * If there's an error, report it
	 */
	if (ret < 0) {
		printk ("unregister_chrdev: error %d\n", ret);
	}
	else {
		printk ("module clean up ok!! \n");
	}	
}
module_init(led_gpio_init);
module_exit(led_gpio_exit);
