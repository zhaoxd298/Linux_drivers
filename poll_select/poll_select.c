#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/semaphore.h>		// semaphore
#include <linux/wait.h>				// wait_queue_head_t
#include <linux/sched.h>
#include <linux/poll.h>


#define MEM_SIZE 4095

struct fifo_dev                                     
{                                                        
	char *buf;             		// 缓冲区         
	unsigned long buf_size;  	// 缓冲区总大小
	unsigned long wt;			// 写指针
	unsigned long rd;			// 读指针
	struct semaphore sem;		// 信号量
	wait_queue_head_t rd_qeue;	// 读操作等待队列
	wait_queue_head_t wr_qeue;	// 写操作等待队列
};

struct fifo_dev *fifo_devp;

static int my_fifo_open(struct inode *inode, struct file *filp)
{
	filp->private_data = fifo_devp;		// 将全局变量mem_devp赋给filp->private_data，减少对全局变量的依赖

	printk("<0> open dev:%d\n", iminor(inode));

	return 0;
}

static ssize_t  my_fifo_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
    int read_size = 0;
    struct fifo_dev *devp = filp->private_data;
    int remain_data = 0;
   
    if (down_interruptible(&devp->sem)) {		/*sem P操作*/
    	return -ERESTARTSYS;
    }

    while (devp->rd == devp->wt) {	/*当前没有数据可读*/
    	up(&devp->sem);

    	if (filp->f_flags & O_NONBLOCK) {	/*非阻塞读，直接返回*/
    		return -EAGAIN;
    	}

    	if (wait_event_interruptible(devp->rd_qeue, (devp->rd != devp->wt))) {
    		return -ERESTARTSYS;	/*等待被信号唤醒了*/
    	}

    	/*继续循环等待数据，先获取锁*/
    	if (down_interruptible(&devp->sem)) {		
	    	return -ERESTARTSYS;
	    }
    }

    //printk("<0> rd:[%d:%d]\n", devp->rd, devp->wt);

    if (devp->wt > devp->rd) {
    	remain_data = devp->wt - devp->rd;
    } else {
    	remain_data = devp->buf_size - (devp->rd - devp->wt); 
    }

    if (count > remain_data) {
		read_size = remain_data;
	} else {
		read_size = count;
	}

	if (devp->wt > devp->rd) {
		copy_to_user(buffer, devp->buf + devp->rd, read_size);
		devp->rd += read_size;
	} else {
		int right_data_size = 0;
		int left_data_size = 0;

		right_data_size = devp->buf_size - devp->rd;

		if (right_data_size >= count) {
			right_data_size = count;
			left_data_size = 0;    			
		} else {
			left_data_size = read_size - right_data_size;
		}

		if (0 == left_data_size) {
			copy_to_user(buffer, devp->buf + devp->rd, read_size);
			devp->rd += read_size;
	    	if (devp->buf_size == devp->rd) {
	    		devp->rd = 0;
	    	}
		} else {
			copy_to_user(buffer, devp->buf + devp->rd, right_data_size);
			copy_to_user(buffer + right_data_size, devp->buf, left_data_size);
			devp->rd = left_data_size;
		}
	}

    up(&devp->sem);

    wake_up_interruptible(&devp->wr_qeue);

	//printk("<0> char_read to user:%s\n", buffer);
	return read_size;
}

/*
* get fifo remain space
*/
static int get_fifo_remain_space(struct fifo_dev *devp)
{
	int remain_space = 0;

	if (NULL == devp) {
		return -1;
	}

	if (devp->wt >= devp->rd) {
		remain_space = devp->buf_size - (devp->wt - devp->rd);
	} else {
		remain_space = devp->rd - devp->wt;
	}

	return remain_space;
}

static ssize_t  my_fifo_write(struct file *filp, const char __user *buffer, size_t count, loff_t* ppos)
{
    struct fifo_dev *devp = filp->private_data;
	int write_size;
	int remain_space;
	int right_space = 0;
    int left_space = 0;

	if (down_interruptible(&devp->sem)) {		/*sem P操作*/
    	return -ERESTARTSYS;
    }

    while (get_fifo_remain_space(devp) <= count) {	/*当前没有空间可写*/
    	up(&devp->sem);

    	if (filp->f_flags & O_NONBLOCK) {		/*非阻塞写，直接返回*/
    		return -EAGAIN;
    	}

    	if (wait_event_interruptible(devp->wr_qeue, (get_fifo_remain_space(devp) > count))) {
    		return -ERESTARTSYS;	/*等待被信号唤醒了*/
    	}

    	/*继续循环等待数据，先获取锁*/
    	if (down_interruptible(&devp->sem)) {		
	    	return -ERESTARTSYS;
	    }
    }


	// 要保留一个字节的空间，若完全写满，即wr==rd时会被认为缓冲区为空
    remain_space = get_fifo_remain_space(devp) - 1;		
    if (remain_space >= count) {
    	write_size = count;
    } else {
    	write_size = remain_space;
    }

    if (devp->wt >= devp->rd) {
    	right_space = devp->buf_size - devp->wt;
    	if (right_space >= write_size) {
    		right_space = count;
    		left_space = 0;
    	} else {
    		left_space = write_size - right_space;
    	}

    	if (0 == left_space) {
    		copy_from_user(devp->buf + devp->wt, buffer, write_size);
    		devp->wt += write_size;
    		if (devp->wt == devp->buf_size) {
    			devp->wt = 0;
    		}
    	} else {
    		copy_from_user(devp->buf + devp->wt, buffer, right_space);
    		copy_from_user(devp->buf, buffer + right_space, left_space);
    		devp->wt = left_space;
    	}
    } else {
    	copy_from_user(devp->buf + devp->wt, buffer, write_size);
    	devp->wt += write_size;
    }

	up(&devp->sem);

    wake_up_interruptible(&devp->rd_qeue);
	
	//printk("<0> char_write from user:%s ppos:%d\n", (struct fifo_dev*)(file->private_data)->buf, (int)*ppos);
	return write_size;
}

static unsigned int my_fifo_poll(struct file *filp, poll_table *wait)
{
	struct fifo_dev *dev = filp->private_data;
	unsigned int mask = 0;

	/*
	* The buffer is circular; it is considered full
	* if "wp" is right behind "rp" and empty if the
	* two are equal. 
	*/
	down(&dev->sem);
	poll_wait(filp, &dev->rd_qeue,  wait);
	poll_wait(filp, &dev->wr_qeue, wait);

	if (dev->rd != dev->wt)
		mask |= POLLIN | POLLRDNORM;  /* readable */

	if (get_fifo_remain_space(dev))
		mask |= POLLOUT | POLLWRNORM;  /* writable */
	up(&dev->sem);
	
	return mask;
}

static int  my_fifo_release(struct inode *inode, struct file *filp)
{
	printk(KERN_EMERG"close dev:%d\n", MINOR(inode->i_rdev));

	return 0;
}

static struct file_operations char_driver_fops = {
	.owner =	THIS_MODULE,
	.open =		my_fifo_open,
	.write =	my_fifo_write,	
	.read =		my_fifo_read,	
	.release =	my_fifo_release,
	.poll = 	my_fifo_poll,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "poll_select_dev",
	.fops = &char_driver_fops,
};

static struct fifo_dev* alloc_fifo_dev(void)
{
	struct fifo_dev* devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!devp) {	/*申请失败*/
		return NULL;
	}   

	memset(devp, 0, sizeof(struct fifo_dev));

	devp->buf = kmalloc(MEM_SIZE, GFP_KERNEL);
	if (NULL == devp->buf) {
		kfree(devp);
		return NULL;
	}

	devp->buf_size = MEM_SIZE;
	devp->wt = 0;
	devp->rd = 0;

	sema_init(&devp->sem, 1);

	init_waitqueue_head(&devp->rd_qeue);
	init_waitqueue_head(&devp->wr_qeue);

	return devp; 
}

static void release_fifo_dev(struct fifo_dev** devp)
{
	if (NULL == devp) {
		return;
	}

	kfree((*devp)->buf);
	(*devp)->buf = NULL;

	kfree(*devp);
	*devp = NULL;
}

static int __init my_fifo_init(void)
{
	int ret;

	fifo_devp = alloc_fifo_dev();
	if (!fifo_devp)    /*申请失败*/
	{
		printk(KERN_EMERG"alloc fifo_dev error!\n");
		return - ENOMEM;
	}

	ret = misc_register(&misc);
	printk("<0> hello driver dev_init!\n");

    return ret;
}

static void __exit my_fifo_exit(void)
{
	release_fifo_dev(&fifo_devp);

	misc_deregister(&misc);
	printk("<0> hello driver dev_exit!\n");
}

MODULE_LICENSE("GPL");
module_init(my_fifo_init);
module_exit(my_fifo_exit);