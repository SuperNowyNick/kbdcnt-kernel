#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>

#include "kbdcnt.h"

static long int kbdKeyCount = 0;
static ktime_t kbdResetTime = 0;

static int kbdMajor = 0;
static struct class *kbdClass;
static struct device *kbdDevice;


static int kbdDeviceCount = 0;

static irqreturn_t kbdIrqHandler(int irq, void *dev_id, struct pt_regs *regs)
{
        unsigned char scancode = inb(0x60);
        if (scancode < 128)
                kbdKeyCount++;
        return (irqreturn_t)IRQ_HANDLED; 
}

static long kbdDeviceIoctl(struct file *file, unsigned int num,
                unsigned long param)
{
        signed long long time;
        switch (num)
        {
        case IOCTL_RST:
                kbdKeyCount = 0;
                kbdResetTime = ktime_get_real();
                break;
        case IOCTL_GET_CNT:
                if (copy_to_user((unsigned long long*)param, &kbdKeyCount,
                                        sizeof(unsigned long long)))
                return -EFAULT;
                break;
        case IOCTL_GET_TIME:
                time = ktime_to_ns(kbdResetTime);
                if (copy_to_user((signed long long*)param, &time,
                                        sizeof(signed long long)))
                return -EFAULT;
                break;
        }
        return 0;
}

static int kbdDeviceOpen(struct inode *inode, struct file *file)
{
        if (kbdDeviceCount)
                return -EBUSY;
        kbdDeviceCount++;
        try_module_get(THIS_MODULE);
        return 0;
}

static int kbdDeviceRelease(struct inode *inode, struct file *file)
{
        kbdDeviceCount--;
        module_put(THIS_MODULE);
        return 0;
}

struct file_operations kbdFileOps = {
        .open = kbdDeviceOpen,
        .release = kbdDeviceRelease,
        .unlocked_ioctl = kbdDeviceIoctl, 
};

static int __init kbdInit(void)
{
        int ret = 0;
        kbdKeyCount = 0;
        kbdResetTime = ktime_get_real();
        kbdMajor = register_chrdev(0, KBDCNT_NAME, &kbdFileOps);
        if (kbdMajor < 0) {
                printk(KERN_ALERT"Failed to register chrdev KeyboardCounter\n");
                return kbdMajor;
        }
        kbdClass = class_create(THIS_MODULE, KBDCNT_CLASS);
        if (IS_ERR(kbdClass)) {
                printk(KERN_ALERT"Failed to create device class KeyboardCounter\n");
                unregister_chrdev(kbdMajor, KBDCNT_NAME);
                return PTR_ERR(kbdClass);
        }
        kbdDevice = device_create(kbdClass, NULL, MKDEV(kbdMajor, 0),
                        NULL, KBDCNT_DEVICE);
        if (IS_ERR(kbdDevice)) {
                printk(KERN_ALERT"Failed to create device KeyboardCounter\n");
                class_destroy(kbdClass);
                unregister_chrdev(kbdMajor, KBDCNT_NAME);
                return PTR_ERR(kbdDevice);
	}
        ret = request_irq(1, (irq_handler_t)kbdIrqHandler, IRQF_SHARED,
                        KBDCNT_DEVICE, (void*)(kbdIrqHandler));
        if (ret < 0) {
                printk(KERN_ALERT"Failed to register KeyboardCounter interrupt\n");
                device_destroy(kbdClass, kbdMajor);
                class_destroy(kbdClass);
                unregister_chrdev(kbdMajor, KBDCNT_NAME);
                return ret;
        }

        printk(KERN_INFO"Loaded KeyboardCounter with major number: %d\n",
                        kbdMajor);
        return 0;
}

static void __exit kbdExit(void)
{
        free_irq(1, (void*)(kbdIrqHandler));
        device_destroy(kbdClass, MKDEV(kbdMajor, 0));
        class_unregister(kbdClass);
        class_destroy(kbdClass);
        unregister_chrdev(kbdMajor, KBDCNT_NAME);
}

module_init(kbdInit);
module_exit(kbdExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grzegorz Sobczak");
MODULE_DESCRIPTION("Kernel module for counting key press interrupts. See source for ioctl documentation");
