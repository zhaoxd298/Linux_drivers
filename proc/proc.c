#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> 
#include <linux/sched.h>
#include <asm/uaccess.h>

#define PROCFS_NAME "procfs"

static ssize_t proc_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 6, ret;
	static int flag = 0;
	
	ret = copy_to_user(buf, "hello\n", len);

	if (flag)
	{
		len = 0;
	}

	flag = !flag;

	pr_err("<%s:%d> %d %d\n", __func__, __LINE__, flag, len);

	return len;
}

static ssize_t proc_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
	int ret, rd_size = 0;
	char tmp_buf[64] = {0};

	while (len > 0) {
		if (len >= sizeof(tmp_buf)) {
			rd_size = sizeof(tmp_buf) - 1;
		} else {
			rd_size = len;
		}

		memset(tmp_buf, 0, sizeof(tmp_buf));
		ret = copy_from_user(tmp_buf, buf + *ppos, sizeof(tmp_buf));
		if (ret) {
			return -EFAULT;
		}

		len -= rd_size;
		*ppos += rd_size;
		pr_err("<%s:%d> %s\n", __func__, __LINE__, tmp_buf);
	}
	

	return *ppos;
}

static  const struct proc_ops proc_ops = {
	.proc_read = proc_read,
	.proc_write = proc_write,
};

int __init procfs_init(void)
{
	int ret = 0;
	if (proc_create(PROCFS_NAME, 0666, NULL, &proc_ops) == NULL) {
		ret = -1;
		printk("create /proc/%s failed.\n", PROCFS_NAME);
	} else {
		printk("create /proc/%s sucess.\n", PROCFS_NAME);
	}

	return 0;
}

void __exit procfs_exit(void)
{
	remove_proc_entry(PROCFS_NAME, NULL);
	printk("Remove /proc/%s sucess.\n", PROCFS_NAME);
}

MODULE_LICENSE("GPL");
module_init(procfs_init);
module_exit(procfs_exit);
