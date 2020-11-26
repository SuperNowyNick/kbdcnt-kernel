#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
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

static long kbdDeviceIoctl(struct file* file, unsigned int num, unsigned long param)
{
	signed long long time;
	switch(num)
	{
		case IOCTL_RST:
			kbdKeyCount = 0;
			kbdResetTime = ktime_get_real();
			break;
		case IOCTL_GET_CNT:
			copy_to_user((unsigned long long*)param, &kbdKeyCount, sizeof(unsigned long long));
			break;
		case IOCTL_GET_TIME:
			time = ktime_to_ns(kbdResetTime);
			copy_to_user((signed long long*)param, &time, sizeof(signed long long));
			break;
	}
}

static int kbdDeviceOpen(struct inode* inode, struct file* file)
{
	if(kbdDeviceCount)
		return -EBUSY;
       	kbdDeviceCount++;	
	try_module_get(THIS_MODULE);
	return 0;
}

static int kbdDeviceRelease(struct inode* inode, struct file* file)
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
	kbdKeyCount = 0;
	kbdResetTime = ktime_get_real();
	kbdMajor = register_chrdev(0, KBDCNT_NAME, &kbdFileOps);
	if(kbdMajor<0)
	{
		printk(KERN_ALERT"Failed to register chrdev KeyboardCounter\n");
		return kbdMajor;
	}
	kbdClass = class_create(THIS_MODULE, KBDCNT_CLASS);
	if(IS_ERR(kbdClass))
	{
		printk(KERN_ALERT"Failed to create device class KeyboardCounter\n");
		return ERR_PTR(kbdClass);
	}
	kbdDevice = device_create(kbdClass, NULL, kbdMajor, NULL, KBDCNT_DEVICE);
	if(IS_ERR(kbdDevice))
	{
		printk(KERN_ALERT"Failed to create device KeyboardCounter\n");
		return ERR_PTR(kbdDevice);
	}

	return 0;
}

static void __exit kbdExit(void)
{
	unregister_chrdev(kbdMajor, KBDCNT_NAME);
}

module_init(kbdInit);
module_exit(kbdExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grzegorz Sobczak");
MODULE_DESCRIPTION("Kernel module for counting key press interrupts. See source for ioctl documentation");
