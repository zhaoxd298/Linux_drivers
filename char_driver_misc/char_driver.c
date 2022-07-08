#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#define MEM_SIZE 4096

struct mem_dev                                      
{                                                        
	char *buf;                      
	unsigned long buf_size;  
	//struct cdev cdev;      
};

struct mem_dev *mem_devp;

static int char_driver_open(struct inode *inode, struct file *filp)
{
	filp->private_data = mem_devp;		// 将全局变量mem_devp赋给filp->private_data，减少对全局变量的依赖

	printk("<0> open dev:%d\n", iminor(inode));

	return 0;
}

static ssize_t  char_driver_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
    int read_size;
    struct mem_dev *my_dev;
    
    my_dev = filp->private_data;
	
	if (count > my_dev->buf_size - *ppos) {
		read_size = my_dev->buf_size - *ppos;
	} else {
		read_size = count;
	}

	copy_to_user(buffer, my_dev->buf + *ppos, read_size);
	*ppos += read_size;

	//printk("<0> char_read to user:%s\n", buffer);
	return read_size;
}

static ssize_t  char_driver_write(struct file *filp, const char __user *buffer, size_t count, loff_t* ppos)
{
    struct mem_dev *my_dev;
	int write_size;

	my_dev = filp->private_data;

	if (count > my_dev->buf_size - *ppos) {
		write_size = my_dev->buf_size - *ppos;
	} else {
		write_size = count; 
	}

	copy_from_user(my_dev->buf + filp->f_pos, buffer, write_size);
	*ppos += count;
	
	//printk("<0> char_write from user:%s ppos:%d\n", (struct mem_dev*)(file->private_data)->buf, (int)*ppos);
	return write_size;
}

static loff_t char_driver_llseek(struct file *filp, loff_t offset, int whence)
{ 
    loff_t newpos;
    struct mem_dev *my_dev = filp->private_data;

    switch(whence) {
    case 0: /* SEEK_SET */
        newpos = offset;
        break;
    case 1: /* SEEK_CUR */
        newpos = filp->f_pos + offset;
        break;
    case 2: /* SEEK_END */
        newpos = my_dev->buf_size -1 + offset;
        break;
    default: /* can't happen */
        return -EINVAL;
    }

    if ((newpos < 0) || (newpos > MEM_SIZE)) {
		return -EINVAL;
    }
        
    filp->f_pos = newpos;

    return newpos;
}

static int  char_driver_release(struct inode *inode, struct file *filp)
{
	printk(KERN_EMERG"close dev:%d\n", MINOR(inode->i_rdev));

	return 0;
}

static struct file_operations char_driver_fops = {
	.owner =	THIS_MODULE,
	.open =		char_driver_open,
	.write =	char_driver_write,	
	.read =		char_driver_read,	
	.llseek =	char_driver_llseek,
	.release =	char_driver_release,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "char_driver",
	.fops = &char_driver_fops,
};

static struct mem_dev* alloc_mem_dev(void)
{
	struct mem_dev* devp = kmalloc(sizeof(struct mem_dev), GFP_KERNEL);
	if (!devp) {	/*申请失败*/
		return NULL;
	}   

	memset(devp, 0, sizeof(struct mem_dev));

	devp->buf = kmalloc(MEM_SIZE, GFP_KERNEL);
	if (NULL == devp->buf) {
		kfree(devp);
		return NULL;
	}

	devp->buf_size = MEM_SIZE;

	return devp; 
}

static void release_mem_dev(struct mem_dev** devp)
{
	if (NULL == devp) {
		return;
	}

	kfree((*devp)->buf);
	(*devp)->buf = NULL;

	kfree(*devp);
	*devp = NULL;
}

static int __init char_driver_init(void)
{
	int ret;

	mem_devp = alloc_mem_dev();
	if (!mem_devp)    /*申请失败*/
	{
		printk(KERN_EMERG"alloc mem_dev error!\n");
		return - ENOMEM;
	}

	ret = misc_register(&misc);
	printk("<0> hello driver dev_init!\n");

    return ret;
}

static void __exit char_driver_exit(void)
{
	release_mem_dev(&mem_devp);

	misc_deregister(&misc);
	printk("<0> hello driver dev_exit!\n");
}

MODULE_LICENSE("GPL");
module_init(char_driver_init);
module_exit(char_driver_exit);