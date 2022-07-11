#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/i2c.h>

#define DEVICE_NAME "emc2301_fan"
#define CLASS_NAME "emc2301_fan_class"

static struct class *emc2301_class;
static struct cdev emc2301_dev;
dev_t dev;

/* 1. Declare a file_operations structure with some operations. */
static int emc2301_dev_open(struct inode *inode, struct file *file)
{
	pr_info("emc2301_dev_open() is called.\n");
	return 0;
}

static int emc2301_dev_close(struct inode *inode, struct file *file)
{
	pr_info("emc2301_dev_close() is called.\n");
	return 0;
}

ssize_t emc2301_dev_read(struct file *file, char __user *user, size_t s, loff_t *l)
{
	u8 ret; 
	struct i2c_client *my_client; 
	struct i2c_adapter *my_adap = i2c_get_adapter(10); // 10 means i2c-10 bus

	pr_info("read called\n");

	my_client = i2c_new_dummy_device(my_adap, 0x2f); // 0x2f - slave address on i2c bus
	ret = i2c_smbus_read_byte_data(my_client, 0x32); // read fan speed

	i2c_unregister_device(my_client);
	pr_info("fan device closed. \n");

	return ret;
}

ssize_t emc2301_dev_write(struct file *file, const char __user *user, size_t speed, loff_t *l)
{

	u8 ret; 
	u8 speedu8 = speed & 0xFF;
	struct i2c_client *my_client; 
	struct i2c_adapter *my_adap = i2c_get_adapter(10); // 10 means i2c-10 bus

	pr_info("write called\n");

	if (!my_adap) {
		pr_err("failed to get i2c adapter 10\n");
		return -EINVAL;
	}


	my_client = i2c_new_dummy_device(my_adap, 0x2f); // 0x2f - slave address on i2c bus
	pr_info("i2c device created. \n");
	ret = i2c_smbus_write_byte_data(my_client, 0x30, speedu8); // set fan speed
	pr_info("i2c write command ret = %d\n", ret);

	i2c_unregister_device(my_client);
	pr_info("fan device closed. \n");

	return ret;
}

static const struct file_operations emc2301_dev_fops = {
	.owner = THIS_MODULE,
	.open = emc2301_dev_open,
	.release = emc2301_dev_close,
	.read = emc2301_dev_read,
	.write = emc2301_dev_write,
};

/* 2. Define a module. */
static int __init emc2301_dev_init(void)
{
	int ret;
	dev_t dev_no;
	int Major;
	struct device* emc2301Device;
	
	/* Allocate device numbers dynamically */
	pr_info("emc2301_dev init.\n");
	ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_info("Unable to allocate Major number.\n");
		return ret;
	}

	/* Get the device identifiers */
	Major = MAJOR(dev_no);
	dev = MKDEV(Major, 0);
	pr_info("Allocated correctly with major number %d.\n", Major);

	/* Initialize the cdev structure and add it to kernel space */
	cdev_init(&emc2301_dev, &emc2301_dev_fops);
	ret = cdev_add(&emc2301_dev, dev, 1);
	if (ret < 0) {
		unregister_chrdev_region(dev, 1);
		pr_info("Unable to add cdev.\n");
		return ret;
	}

	/* Register the device class */
	emc2301_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(emc2301_class)) {
		unregister_chrdev_region(dev, 1);
		cdev_del(&emc2301_dev);
		pr_info("Failed to register device class\n");
		return PTR_ERR(emc2301_class);
	}
	pr_info("device class registered correctly.\n");

	/* Create a device node named DEVICE_NAME associated to dev */
	emc2301Device = device_create(emc2301_class, NULL, dev, NULL, DEVICE_NAME);
	if (IS_ERR(emc2301Device)) {
		class_destroy(emc2301_class);
		cdev_del(&emc2301_dev);
		unregister_chrdev_region(dev, 1);
		pr_info("Failed to create the device\n");
		return PTR_ERR(emc2301Device);
	}
	pr_info("The device is created correctly\n");
	return 0;
}

static void __exit emc2301_dev_exit(void)
{
	device_destroy(emc2301_class, dev);  /* remove the device */
	class_destroy(emc2301_class);        /* remove the device class */
	cdev_del(&emc2301_dev);
	unregister_chrdev_region(dev, 1); /* unregister the device numbers */
	pr_info("emc2301 driver exit\n");
}

module_init(emc2301_dev_init);
module_exit(emc2301_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bin Zeng");
MODULE_DESCRIPTION("The code is copied and modified from the book: Linux Driver Development with Raspberry Pi - Practical Labs.");

