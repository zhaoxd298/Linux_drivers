#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static char buf0[200] = {0};
static char buf1[200] = {0};

/*
struct resource {
        resource_size_t start;
        resource_size_t end;
        const char *name;
        unsigned long flags;
        unsigned long desc;
        struct resource *parent, *sibling, *child;
};
*/

/*
static struct resource pdev_resources[] = {
	[0] = {
		.start = (resource_size_t )buf0,
		.end = (resource_size_t )(buf0 + sizeof(buf0)-1),
		.name = "pdev0-resource", 
		.flags = IORESOURCE_MEM,
		.desc = IORES_DESC_NONE,   
		},
	[1] = {
		.start = (resource_size_t )buf1,
		.end = (resource_size_t )(buf1 + sizeof(buf1)-1),
		.name = "pdev1-resource", 
		.flags = IORESOURCE_MEM,
		.desc = IORES_DESC_NONE,   
		},
};
*/

static void pdev_release(struct device *dev)
{
}

struct platform_device pdev0 = {
	.name = "pdev",
	.id = 0,
	.num_resources = 0,
	.resource = NULL,
	.dev = {
		.release = pdev_release,
	},
}; 

struct platform_device pdev1 = {
	.name = "pdev",
	.id = 1,
	.num_resources = 0,
	.resource = NULL,
	.dev = {
		.release = pdev_release,
	},
};

static int __init pltdev_init(void)
{
	//strcpy(buf0, "hello");
	//strcpy(buf1, "world");

	pr_err("<%s:%d> pltdev_init!\n", __func__, __LINE__);
	
	platform_device_register(&pdev0);
	platform_device_register(&pdev1);

	return 0;
}

static void __exit pltdev_exit(void)
{
	pr_err("<%s:%d> pltdev_exit!\n", __func__, __LINE__);
	
	platform_device_unregister(&pdev1);
	platform_device_unregister(&pdev0);
}

module_init(pltdev_init);
module_exit(pltdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ahren.zhao");
MODULE_DESCRIPTION("register a platfom device");
