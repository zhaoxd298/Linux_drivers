#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/platform_device.h>

static int pdrv_suspend(struct device *dev)
{
	pr_err("<%s:%d>\n", __func__, __LINE__);
	return 0;
}

static int pdrv_resume(struct device *dev)
{
	pr_err("<%s:%d>\n", __func__, __LINE__);
	return 0;
}

static const struct dev_pm_ops pdrv_pm_ops = {
	.suspend = pdrv_suspend,
	.resume  = pdrv_resume,
};

static int pdrv_probe(struct platform_device *pdev)
{
	//struct resource* mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	//pr_err("id:%d name:%s buf:%s\n", pdev->id, mem_res->name, (char*)mem_res->start);
	
	pr_err("<%s:%d> id:%d\n", __func__, __LINE__, pdev->id);

	return 0;
}

static int pdrv_remove(struct platform_device *pdev)
{
	pr_err("<%s:%d>\n", __func__, __LINE__);

	return 0;
}

struct platform_driver pdrv = {
	.driver = {
		.name    = "pdev",
		.owner   = THIS_MODULE,
		.pm      = &pdrv_pm_ops,
	},
	.probe   = pdrv_probe,
	.remove  = pdrv_remove,
};

static int __init pdev_init(void)
{
	int ret;

	printk(KERN_EMERG"pdev_init!\n");
	ret = platform_driver_register(&pdrv);

	return ret;
}

static void __exit pdev_exit(void)
{
	printk(KERN_EMERG"pdev_exit!\n");
	
	platform_driver_unregister(&pdrv);
}

module_init(pdev_init);
module_exit(pdev_exit);

//module_platform_driver(pdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiaodongzhao");
MODULE_DESCRIPTION("A simple platform driver");
MODULE_ALIAS("platform:pdev");
