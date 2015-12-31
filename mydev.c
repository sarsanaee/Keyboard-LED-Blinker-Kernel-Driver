#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/types.h>   // for dev_t typedef
#include <linux/kdev_t.h>  // for format_dev_t
#include <linux/fs.h>      // for alloc_chrdev_region()
#include <asm/uaccess.h>
#include <linux/leds.h>
#include <linux/list.h>


//////////////////////////////////


#include <linux/configfs.h>
#include <linux/tty.h>          /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>           /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>       /* For vc_cons */
#include <linux/vt_kern.h>


struct timer_list my_timer;
struct tty_driver *my_driver;
char kbledstatus = 0;
#define BLINK_DELAY   HZ/1
#define FINISH_BLINK_DELAY HZ*2
#define ZERO 0x00
#define ONE 0x01
#define TWO 0x02
#define THREE 0x03
#define FOUR 0x04
#define FIVE 0x05
#define SIX 0x06
#define SEVEN 0x07
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

#define BUF_MAX_SIZE		1024
static int output[9] = {0,0,0};
static int test = 0;
static int index = 0;
static int toshow[3] = {0,0,0};
static char buff[BUF_MAX_SIZE];
static dev_t mydev;           
struct cdev my_cdev;
struct todo_struct {
    struct list_head list;
    char data;
};
static struct list_head my_list_head;
static struct list_head *ptr_keeper = &my_list_head;


void binary_representation(char input_char)
{
    int i;
    
    printk(KERN_INFO "BINARY REPRESENTATION OF %c\n", input_char);
    for (i = 0; i < 9; ++i)
    {   
        output[i] = 0;
        output[i] = (input_char >> i) & 1;
        printk(KERN_INFO "%d\n", output[i]);

    }    

}


void reset_driver_state(void)
{
    
    test = 0;
    index = 0;

}


void blinker(void)
{

    struct todo_struct *entry;
    ptr_keeper = ptr_keeper->next;
    if(ptr_keeper != &my_list_head)
    {
	entry = list_entry(ptr_keeper, struct todo_struct, list);
	printk(KERN_INFO "Blinker Function");
	binary_representation(entry->data);
	my_timer.expires = jiffies + BLINK_DELAY;
	add_timer(&my_timer);
    }

}


int hex_selector(int * input, int part)
{

    if(input[0] == 0 && input[1] == 0 && input[2] == 0)
	    return ZERO;
    else if(input[0] == 1 && input[1] == 0 && input[2] == 0)
	    return ONE;
    else if(input[0] == 0 && input[1] == 1 && input[2] == 0)
	    return TWO;
    else if(input[0] == 1 && input[1] == 1 && input[2] == 0)
	    return THREE;
    else if(input[0] == 0 && input[1] == 0 && input[2] == 1)
	    return FOUR;
    else if(input[0] == 1 && input[1] == 0 && input[2] == 1)
	    return FIVE;
    else if(input[0] == 0 && input[1] == 1 && input[2] == 1)
	    return SIX;
    else
	    return SEVEN;
		
    
}


static void my_timer_func(unsigned long ptr)
{
    int *pstatus = (int *)ptr;
    
    printk(KERN_INFO "my_timer_function \n");
        
    if(test < 3)
    { 
	    
	toshow[0] = output[index];
	toshow[1] = output[index+1];
	toshow[2] = output[index+2];
	printk(KERN_INFO "int number to show %x\n", hex_selector(toshow,1));
	

	if (*pstatus == hex_selector(toshow,1))
	{
		*pstatus = RESTORE_LEDS;
		printk(KERN_INFO " ALL_LED_ON \n");
	}
	else
	{
		*pstatus = hex_selector(toshow,1);
		printk(KERN_INFO " ALL_LED_RESTORE \n");
	}
   

	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,
			    *pstatus);
	my_timer.expires = jiffies + BLINK_DELAY;
	add_timer(&my_timer);
	test++;
	index = index + 3;
    }
    else if(test == 3)
    {
	*pstatus = ALL_LEDS_ON;
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,
		*pstatus);
	my_timer.expires = jiffies + FINISH_BLINK_DELAY;
	add_timer(&my_timer);
	test++;
    }
    else
    {
	test = 0;
	index = 0;
	blinker();
    }
        
}



/////////////////////




void insert_into_linked_list(int count)
{
    
    struct list_head *ptr;
    struct todo_struct *entry;
    int i = 0;
    printk(KERN_INFO "here in insert_into_linked_list function %d \n", count);
    for(i = 0; i < count; i = i + 1)
    {        
         struct todo_struct *bufferPtr = (struct todo_struct *)vmalloc(sizeof(struct todo_struct));           
         bufferPtr->data = buff[i];
         INIT_LIST_HEAD(&bufferPtr->list);
	 printk(KERN_INFO "In for %d \n", i);
         list_add_tail(&bufferPtr->list, &my_list_head);
    }

    list_for_each(ptr, &my_list_head) 
    {
        entry = list_entry(ptr, struct todo_struct, list);
        printk(KERN_INFO "what is in this linked list :)  %c\n", entry->data);
    }
    
    return;
    
}



ssize_t my_write (struct file *flip, const char __user *buf, size_t count, loff_t *f_ops)
{

    if(count >  BUF_MAX_SIZE)
    {
        copy_from_user(buff, buf, BUF_MAX_SIZE);
        return 1024;
    }
    else
    {
        printk(KERN_INFO "module chardrv being writing.\n"); 
        copy_from_user(buff, buf, count);
        printk(KERN_INFO "BUFFER %s",buff);
        INIT_LIST_HEAD(&my_list_head);
        insert_into_linked_list(count);
        return count;
    }

}

ssize_t my_read(struct file *flip, char __user *buf, size_t count, loff_t *f_ops)
{
	
	

    struct list_head *ptr;
    struct todo_struct *entry;
    int * binary = (int *) vmalloc(sizeof(int));
    reset_driver_state();
    printk(KERN_INFO "Read Function\n");
    copy_to_user(buf, &buff[*f_ops],1);
    if(buff[*f_ops]=='\0')
    {
        return 0;
    }
    
    
    
    
    list_for_each(ptr, &my_list_head) 
    {
        
        entry = list_entry(ptr, struct todo_struct, list);
        printk(KERN_INFO "data =   %c\n", entry->data);
    
    }
    vfree(binary);
    blinker();
    
    return 0; 
    ///////////////////////////
    *f_ops+=1;
    return 1;

}


struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write=my_write,
};

static int __init chardrv_in(void)
{
    printk(KERN_INFO "module chardrv being loaded.\n");

    alloc_chrdev_region(&mydev, 0, 1, "eadriver");
    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;
    cdev_add(&my_cdev, mydev, 1);
    
//////////////////////////////////////////////////////////
    my_driver = vc_cons[fg_console].d->port.tty->driver;
    printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);
        /*
         * Set up the LED blink timer the first time
         */
    init_timer(&my_timer);
    my_timer.function = my_timer_func;
    my_timer.data = (unsigned long)&kbledstatus;
    my_timer.expires = jiffies + BLINK_DELAY;
    
//////////////////////////////////////////////////////////
    return 0;
}

static void __exit chardrv_out(void)
{
    printk(KERN_INFO "module chardrv being unloaded.\n");
    cdev_del(&my_cdev);
    unregister_chrdev_region(mydev, 1);
    del_timer(&my_timer);
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,
			RESTORE_LEDS);
}

module_init(chardrv_in);
module_exit(chardrv_out);

MODULE_AUTHOR("Seyed Alireza Sanaee");
MODULE_LICENSE("GPL");
