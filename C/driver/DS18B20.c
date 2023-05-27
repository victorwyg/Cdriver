#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/major.h>
#include <linux/efi.h>
#include <linux/wait.h>
#include <linux/timer.h>

#define DEV_NAME ("ds18_1")
#define DEV_CNT (1)

#define CMD_SKIP_ROM (0xcc)
#define CMD_CONVERT_TEMP (0x44)
#define CMD_READ_SCRATCHPAD (0xbe)

struct ds18b20_dev
{
    struct cdev cdev;
    struct class *ds18_class;
    struct device_node *dev_node;
    struct device *ds18_device;
    dev_t devid;
    int dev_pin;

    bool device_open;
    spinlock_t lock;
};

static struct
    ds18b20_dev *temp_dev = NULL;

static int
ds18b20_init(void)
{
    int ret = 0;
    unsigned long flags;

    gpio_direction_output(temp_dev->dev_pin, 1);
    udelay(60);
    gpio_set_value(temp_dev->dev_pin, 0);
    udelay(480);
    gpio_set_value(temp_dev->dev_pin, 1);
    udelay(65);
    spin_lock_irqsave(&temp_dev->lock, flags);
    gpio_direction_input(temp_dev->dev_pin);
    ret = gpio_get_value(temp_dev->dev_pin);
    spin_unlock_irqrestore(&temp_dev->lock, flags);
    return ret;
} /*传感器初始化，发送复位脉冲*/

static void
ds18b20_write8(unsigned char data)
{
    int i;
    unsigned long flags;

    spin_lock_irqsave(&temp_dev->lock, flags);
    gpio_direction_output(temp_dev->dev_pin, 1);
    for (i = 0; i < 8; ++i)
    {
        gpio_set_value(temp_dev->dev_pin, 0);
        udelay(1);
        if (data & 0x01)
        {
            gpio_set_value(temp_dev->dev_pin, 1);
        }
        data >>= 1;
        udelay(60);
        gpio_set_value(temp_dev->dev_pin, 1);
        udelay(16);
    }
    gpio_set_value(temp_dev->dev_pin, 1);
    spin_unlock_irqrestore(&temp_dev->lock, flags);
}

static unsigned char
ds18b20_read8(void)
{
    int i;
    unsigned char ret = 0;
    unsigned long flags;

    for (i = 0; i < 8; ++i)
    {
        spin_lock_irqsave(&temp_dev->lock, flags);
        gpio_direction_output(temp_dev->dev_pin, 0);
        udelay(1);
        gpio_set_value(temp_dev->dev_pin, 1);

        gpio_direction_input(temp_dev->dev_pin);
        ret >>= 1;
        if (gpio_get_value(temp_dev->dev_pin))
        {
            ret |= 0x80;
        }
        spin_unlock_irqrestore(&temp_dev->lock, flags);
        udelay(60);
    }

    gpio_direction_output(temp_dev->dev_pin, 1);

    return ret;
}

static int
ds18b20_open(struct inode *inode, struct file *filp)
{
    filp->private_data = temp_dev; /* 设置私有数据 */
    if ((temp_dev != NULL) && !temp_dev->device_open)
    {
        temp_dev->device_open = true;
        try_module_get(THIS_MODULE);
    }
    else
        return -EFAULT;
    return 0;
}
static ssize_t
ds18b20_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    struct ds18b20_dev *dev = filp->private_data;
    if (!dev->device_open)
        return -EFAULT;

    if (buf != NULL)
    {
        /* code */
    }

    return 0;
}
static ssize_t
ds18b20_read(struct file *filp, char *buf, size_t len, loff_t *offset)
{
    int ret;
    int lenth;
    unsigned char LSB, MSB;
    char kerBuf[2];
    char userBuf[20];
    struct ds18b20_dev *dev = filp->private_data;
    if (!dev->device_open)
        return -EFAULT;

    if (ds18b20_init() != 0)
    {
        ret = -EFAULT;
    }

    udelay(420);
    ds18b20_write8(CMD_SKIP_ROM);     // 跳过ROM寻址
    ds18b20_write8(CMD_CONVERT_TEMP); // This command begins a temperature conversion

    mdelay(750);
    if (ds18b20_init() != 0)
    {
        ret = -EFAULT;
    }

    udelay(400);
    ds18b20_write8(CMD_SKIP_ROM);        // 跳过ROM寻址
    ds18b20_write8(CMD_READ_SCRATCHPAD); // This command copies the scratchpad into the E2 memory of the DS18B20, storing the temperature trigger bytes in nonvolatile memory.

    LSB = ds18b20_read8();
    MSB = ds18b20_read8();

    *(kerBuf + 0) = (LSB >> 4) | (MSB << 4);

    *(kerBuf + 1) = (LSB & 0x0f) * 1000 >> 4;

    // ret = copy_to_user(buf, kerBuf, sizeof(kerBuf));
    sprintf((char *)userBuf, "%d.%dC\r\n", kerBuf[0], kerBuf[1]);
    // printk(KERN_EMERG "%s", userBuf);
    lenth = strlen(userBuf);
    ret = copy_to_user(buf, userBuf, lenth);
    // printk(KERN_EMERG "%d.%d C\r\n", kerBuf[0], kerBuf[1]);
    if (ret)
    {
        ret = -EFAULT;
    }

    return lenth;
}

static int
ds18b20_release(struct inode *inode, struct file *filp)
{
    struct ds18b20_dev *dev = filp->private_data;
    if (dev->device_open == true)
    {
        dev->device_open = false;
        module_put(THIS_MODULE);
    }
    return 0;
}

static long
ds18b20_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int nResult = 0;
    struct ds18b20_dev *dev = filp->private_data;
    if (!dev->device_open)
        return -EFAULT;
    switch (cmd)
    {
    case 0:

        break;
    default:
        break;
    }

    return nResult;
}

static struct file_operations ds18b20_fops =
    {
        .owner = THIS_MODULE,
        .open = ds18b20_open,
        .write = ds18b20_write,
        .read = ds18b20_read,
        .release = ds18b20_release,
        .unlocked_ioctl = ds18b20_ioctl,
};

static int
ds18b20_probe(struct platform_device *pdev)
{
    int result = 0;
    temp_dev = kzalloc(sizeof(struct ds18b20_dev), GFP_KERNEL);
    if (!temp_dev)
    {
        result = -ENOMEM;
        goto err_free_mem;
    }
    result = alloc_chrdev_region(&temp_dev->devid, 0, DEV_CNT, DEV_NAME); /*申请设备号*/
    if (result < 0)
        goto alloc_err;

    cdev_init(&temp_dev->cdev, &ds18b20_fops); /*字符设备注册*/

    result = cdev_add(&temp_dev->cdev, temp_dev->devid, DEV_CNT); /*字符设备添加*/
    if (result < 0)
        goto cdevadd_err;

    temp_dev->ds18_class = class_create(THIS_MODULE, "ds18b20_1"); /*创建类*/

    temp_dev->ds18_device = device_create(temp_dev->ds18_class, NULL, temp_dev->devid, NULL, DEV_NAME); /*创建设备*/
    if (temp_dev->ds18_device == NULL)
        goto device_err;
    temp_dev->dev_node = of_find_node_by_path("/Temp1");
    if (temp_dev->dev_node == NULL)
    {

        goto fail2;
    }
    temp_dev->dev_pin = of_get_named_gpio(temp_dev->dev_node, "ds18b20_pin", 0);
    result = gpio_request(temp_dev->dev_pin, "ds18b20_pin");

    printk(KERN_EMERG "ds18b20_probe %d\r\n", result);
    spin_lock_init(&temp_dev->lock);
    printk("\n probe succsed \n");
    return 0;

fail2:
    cdev_del(&temp_dev->cdev);
    printk(KERN_EMERG "of_find_node_by_path failed \r\n");
alloc_err:
    printk("\n devid alloc failed!\n");
cdevadd_err:
    printk("\n cdev add failed!\n");
device_err:
    printk("\n device create failed!\n");
err_free_mem:
    kfree(temp_dev);
    return -1;
}

static int
ds18b20_remove(struct platform_device *pdev)
{
    device_destroy(temp_dev->ds18_class, temp_dev->devid); /*摧毁设备，类+设备号*/
    class_destroy(temp_dev->ds18_class);
    cdev_del(&temp_dev->cdev);
    unregister_chrdev_region(temp_dev->devid, DEV_CNT);
    return 0;
}
static const struct of_device_id ds18_tree[] = {
    {.compatible = "DS18-1"},
    {/*sentinal*/}};
struct platform_driver ds18b20_platdev = {
    .probe = ds18b20_probe,
    .remove = ds18b20_remove,
    .driver = {
        .name = "wyg_driver1",
        .owner = THIS_MODULE,
        .of_match_table = ds18_tree,
    },
};
static int __init platform_init(void)
{
    int error;
    error = platform_driver_register(&ds18b20_platdev);
    printk(KERN_EMERG "\tDriverState = %d\n", error);
    return 0;
}
static void __exit platform_exit(void)
{
    printk(KERN_EMERG "Driver exit!\n");
    platform_driver_unregister(&ds18b20_platdev);
}
module_init(platform_init);
module_exit(platform_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wyg");