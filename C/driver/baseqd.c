#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>

#include <linux/platform_device.h>

#define DEV_CNT (1)
#define DEV_NAME "wygdev"

struct chrdev{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *node;
    int gpio_value;
};

struct chrdev xdev;

static int xxx_open(struct inode *inode,struct file *filp){
    printk("\n open form driver \n");
    return 0;

}


static ssize_t xxx_write(struct file *filp,const char __user *buf,size_t cnt,loff_t * offt){
    
    unsigned char write_data; //用于保存接收到的数据
	int error = copy_from_user(&write_data, buf, cnt);
	if(error < 0) {
		return -1;
	}
    if(write_data & 0x04)
	{
		gpio_set_value(xdev.gpio_value, 0);  // GPIO1_04引脚输出低电平，红灯亮
	}
	else
	{
		gpio_set_value(xdev.gpio_value, 1);    // GPIO1_04引脚输出高电平，红灯灭
	}
    return 0;
}


static struct file_operations xxx_fops = {
    .owner = THIS_MODULE,
    .open = xxx_open,
    .write = xxx_write,
};




static int xxx_probe(struct platform_device *dev){
    int ret = 0;
    
    xdev.node = of_find_node_by_path("/do");
    if(xdev.node == NULL)
    {
        printk(KERN_EMERG "\t  get rgb_led failed!  \n");
    }
    xdev.gpio_value = of_get_named_gpio(xdev.node,"do1",0);
    gpio_request(xdev.gpio_value, "do1");
    gpio_direction_output(xdev.gpio_value,0);
/*---------------------注册 字符设备部分-----------------*/
    ret = alloc_chrdev_region(&xdev.devid,0,DEV_CNT,DEV_NAME);
    if(ret < 0){
        printk("alloc id failed!\n");
        goto alloc_err;
    }
    cdev_init(&xdev.cdev,&xxx_fops);
    ret = cdev_add(&xdev.cdev,xdev.devid,DEV_CNT);
    if(ret < 0){
        printk("chrdev add failed!\n");
        goto add_err;
    }
    xdev.class = class_create(THIS_MODULE,DEV_NAME);
    xdev.device = device_create(xdev.class,NULL,xdev.devid,NULL,DEV_NAME);

    return 0;
    add_err:
    unregister_chrdev_region(xdev.devid,DEV_CNT);
    printk("\n unregister devid \n");
    alloc_err:
    return -1;
}

static int xxx_remove(struct platform_device *dev){
    gpio_free(xdev.gpio_value);
    device_destroy(xdev.class, xdev.devid);
	class_destroy(xdev.class);
	cdev_del(&xdev.cdev);
	unregister_chrdev_region(xdev.devid, DEV_CNT);

    return 0;
}


static const struct of_device_id xxx_match[] = {
    {.compatible = "wyg-driver"},
    {/*sentinel*/}
};
struct platform_driver xxx_driver = {
    .probe = xxx_probe,
    .remove = xxx_remove,
    .driver = {
        .name = "ww",
        .owner = THIS_MODULE,
        .of_match_table = xxx_match,
    }
};

static int __init xxx_init(void)
{
    platform_driver_register(&xxx_driver);
    printk(KERN_EMERG "[ KERN_EMERG ] Hello Module Init\n");
    printk( "[ default ] Hello Module Init\n");
    return 0;
}
static void __exit xxx_exit(void)
{
    platform_driver_unregister(&xxx_driver);
    printk("[ default ] Hello Module Exit\n");
}
module_init(xxx_init);
module_exit(xxx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WYG");