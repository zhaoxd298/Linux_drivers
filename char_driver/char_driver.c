#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>   // 包含内核相关的函数
#include <linux/fs.h>       // 包含文件系统相关的函数
#include <linux/device.h>   // 包含设备相关的函数
#include <linux/version.h>

#define MEM_SIZE 4096

typedef struct mem_dev                                      
{    
	struct cdev dev;
	dev_t devno;
	struct class *devclass;

	char *buf;                      
	unsigned long buf_size;  
} mem_dev_t;

struct mem_dev *mem_devp;

static int char_driver_open(struct inode *inode, struct file *filp)
{
	mem_dev_t *mem_dev = container_of(inode->i_cdev, mem_dev_t, dev);
	if (NULL == mem_dev)
	{
		pr_err("invalid parameter!\n");
		return -1;
	}
	
	filp->private_data = mem_dev;

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

	ret = alloc_chrdev_region(&mem_devp->devno, 0, 1, "char_driver"); 
	if (ret) {
		pr_err("alloc dev-no failed.\n");
		release_mem_dev(&mem_devp);
		return ret;
	}

	cdev_init(&mem_devp->dev, &char_driver_fops);
    mem_devp->dev.owner = THIS_MODULE;
    mem_devp->dev.ops = &char_driver_fops;
    ret = cdev_add(&mem_devp->dev, mem_devp->devno, 1);
    if (ret) {
    	unregister_chrdev_region(mem_devp->devno, 1);
		release_mem_dev(&mem_devp);
        return ret;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    mem_devp->devclass = class_create("char_class");
#else
    mem_devp->devclass = class_create(THIS_MODULE, "char_class");
#endif

	if (IS_ERR(mem_devp->devclass)) 
	{
		printk("class_create failed.\n");
		cdev_del(&mem_devp->dev);
		release_mem_dev(&mem_devp);
		ret = -EIO;
		return ret;
	}
	
    device_create(mem_devp->devclass, NULL, mem_devp->devno, NULL, "char_driver");

	printk("<0> hello driver dev_init!\n");

    return ret;
}

static void __exit char_driver_exit(void)
{
	device_destroy(mem_devp->devclass, mem_devp->devno);
    class_destroy(mem_devp->devclass);
    cdev_del(&mem_devp->dev);
    unregister_chrdev_region(mem_devp->devno, 1);
	release_mem_dev(&mem_devp);

	printk("<0> hello driver dev_exit!\n");
}

MODULE_LICENSE("GPL");
module_init(char_driver_init);
module_exit(char_driver_exit);