#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/version.h>

struct timer_dev                                      
{    
	struct cdev dev;
	dev_t devno;
	struct class *devclass;

	struct timer_list timer;
};

struct timer_dev *timer_dev;
int timer_cnt = 0;

void timer_fun(struct timer_list *timer)
{
	mod_timer(timer, jiffies + HZ);

	pr_err("timer:%d\n", timer_cnt++);
}

static int timer_open(struct inode *inode, struct file *filp)
{	
	pr_err("<%s:%d> open dev:%d\n", __func__, __LINE__, iminor(inode));

	timer_cnt = 0;

	timer_dev->timer.expires = jiffies + HZ;		/*timer duration 1s*/

	timer_setup(&timer_dev->timer, timer_fun, 0);

	add_timer(&timer_dev->timer);

	return 0;
}

static ssize_t timer_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t timer_write(struct file *filp, const char __user *buffer, size_t count, loff_t* ppos)
{
	return 0;
}

static int timer_release(struct inode *inode, struct file *filp)
{
	pr_err("<%s:%d> close dev:%d\n", __func__, __LINE__, iminor(inode));
	del_timer(&timer_dev->timer);

	return 0;
}

static struct file_operations char_driver_fops = {
	.owner =	THIS_MODULE,
	.open =		timer_open,
	.write =	timer_write,	
	.read =		timer_read,	
	.release =	timer_release,
};

static int __init timer_init(void)
{
	int ret;

	timer_dev = kmalloc(sizeof(struct timer_dev), GFP_KERNEL);
	if (!timer_dev) {   /*申请失败*/
		printk(KERN_EMERG"alloc timer_dev error!\n");
		return - ENOMEM;
	}

	ret = alloc_chrdev_region(&timer_dev->devno, 0, 1, "timer_dev"); 
	if (ret) {
		printk("alloc dev-no failed.\n");
		kfree(timer_dev);
		timer_dev = NULL;
		return ret;
	}

	cdev_init(&timer_dev->dev, &char_driver_fops);
	timer_dev->dev.owner = THIS_MODULE;
	timer_dev->dev.ops = &char_driver_fops;
	ret = cdev_add(&timer_dev->dev, timer_dev->devno, 1);
	if (ret) {
		unregister_chrdev_region(timer_dev->devno, 1);
		kfree(timer_dev);
		timer_dev = NULL;
		return ret;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    timer_dev->devclass = class_create("timer_class");
#else
    timer_dev->devclass = class_create(THIS_MODULE, "timer_class");
#endif
	if (IS_ERR(timer_dev->devclass)) 
	{
		printk("class_create failed.\n");
		cdev_del(&timer_dev->dev);
		kfree(timer_dev);
		timer_dev = NULL;
		ret = -EIO;
		return ret;
	}
	
	device_create(timer_dev->devclass, NULL, timer_dev->devno, NULL, "timer_dev");

	printk(KERN_EMERG "timer dev init!\n");

	return ret;
}

static void __exit timer_exit(void)
{
	device_destroy(timer_dev->devclass, timer_dev->devno);
	class_destroy(timer_dev->devclass);
	cdev_del(&timer_dev->dev);
	unregister_chrdev_region(timer_dev->devno, 1);
	kfree(timer_dev);
	timer_dev = NULL;

	printk(KERN_EMERG "timer dev exit!\n");
}

MODULE_LICENSE("GPL");
module_init(timer_init);
module_exit(timer_exit);