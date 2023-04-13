
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>

/*------------------字符设备内容----------------------*/
#define DEV_NAME            "rgb-leds"
#define DEV_CNT                 (1)

typedef struct {
    int red;                                               /*红灯引脚*/
    int green;
    int blue;
    dev_t devid;                                           /*设备号*/
    struct cdev ledcdev;
    struct device_node *node;                              /*设备节点*/
    struct device *device;                                 /*设备*/
    struct class *class;                                   /*类*/

}dev;                                                    /*定义设备结构体*/

dev LED;                                                 /*声明设备*/

static int led_open(struct inode *inode,struct file *filp){
    printk("open form driver\n");
    return 0;
}

static ssize_t led_write(struct file *filp,const char __user *buf,size_t cnt,loff_t *offt){
    unsigned char write_data;

}
static struct file_operations led_chrdev_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
};
static int DO_probe(struct platform_device *dev){
    unsigned int register_data = 0;
    int ret = 0;
    printk("match successed\n");

    LED.node = of_find_node_by_path("/DO");
    if(LED.node == NULL)
    printk(KERN_EMERG"\t get DO node failed!!");
    /*-----获取灯对应引脚-----*/
    LED.red = of_get_named_gpio(LED.node,"red",0);
    LED.green = of_get_named_gpio(LED.node,"green",0);
    LED.blue = of_get_named_gpio(LED.node,"blue",0);
    /*-----设置输入输出方向-----*/
    gpio_direction_output(LED.red,1);
    gpio_direction_output(LED.green,1);
    gpio_direction_output(LED.blue,1);
    /*-----字符设备注册-----*/
    ret = alloc_chrdev_region(&LED.devid,0,DEV_CNT,DEV_NAME);
    inf(ret < 0){
        printk("alloc failed\n");
        goto alloc_err;
    }
    LED.ledcdev.owner = THIS_MODULE;
    cdev_init(&LED.ledcdev,&led_chrdev_fops);
    ret = cdev_add(&LED.ledcdev,LED.devid,DEV_CNT);
    if(ret < 0){
        printk("fail to add cdev\n");
        goto add_err;
    }
    LED.class = class_create(THIS_MODULE,DEV_NAME);
    LED.device = device_create(LED.class,NULL,LED.devid,NULL,DEV_NAME);
    return 0;
    
    add_err:
    unregister_chardev_region(LED.devid,DEV_CNT);
    printk("\n error! \n");
    alloc_err:
    return -1;
}

static const struct of_device_id DO_gpio[] ={
    {.compatible = "wyg,DO"},
    {/*sentinel*/}
};

struct platform_driver DO_platform_driver = {
    .probe = DO_probe,
    .driver = {
            .name = "D0",
            .owner = THIS_MODULE,
            .of_match_table = DO_gpio,
    }
};

static int __init DO_platform_init(void){
    int error;
    error = platform_driver_register(&DO_platform_driver);
    printk(KERN_EMERG "\tDriverState = %d\n",error);
}
static void __exit DO_platform_exit(void){
    printk(KERN_EMERG "Driver exit!\n");
    platform_driver_unregister(&DO_platform_driver);
}
module_init(DO_platform_init);
module_exit(DO_platform_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wyg");