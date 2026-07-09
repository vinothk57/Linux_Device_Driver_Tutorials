#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h> /* Required for manual cdev structures */
#include <linux/uaccess.h> /* Required for copy_to_user and copy_from_user */

/* Global variables for device management */
static dev_t device_number;       /* Holds major and minor numbers */
static struct cdev test_cdev;     /* Character device structure */
static char kernel_buffer[64];   /* Internal buffer for read/write */

/**
 * @brief This function is called when the device file is opened.
 * It demonstrates how to extract major and minor numbers from the inode.
 */
static int my_open(struct inode *device_file, struct file *instance) {
    /* Use macros to extract major and minor numbers from the inode */
    unsigned int major = imajor(device_file);
    unsigned int minor = iminor(device_file);

    /* Log the event using KERN_INFO level */
    printk(KERN_INFO "test_cdev: Device opened (Major: %u, Minor: %u)\n", major, minor);
    return 0;
}

/**
 * @brief This function is called when the device file is closed.
 */
static int my_release(struct inode *device_file, struct file *instance) {
    /* Use pr_info alias for logging the closure */
    pr_info("test_cdev: Device closed\n");
    return 0;
}

/**
 * @brief Handles reading from the kernel buffer to user space.
 */
static ssize_t my_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;

    /* Calculate how many bytes to copy based on buffer size and current offset */
    to_copy = min(count, (size_t)(sizeof(kernel_buffer) - *offs));

    /* If we have reached the end of the buffer, return 0 (EOF) */
    if (to_copy <= 0) {
        return 0;
    }

    /* Transfer data to user space. Returns 0 on success */
    not_copied = copy_to_user(user_buffer, kernel_buffer + *offs, to_copy);
    delta = to_copy - not_copied;

    /* Update the file offset so the next read starts where we left off */
    *offs += delta;

    /* Use KERN_DEBUG for detailed operation logs */
    printk(KERN_DEBUG "test_cdev: Read %d bytes, new offset is %lld\n", delta, *offs);
    return delta;
}

/**
 * @brief Handles writing from user space to the kernel buffer.
 */
static ssize_t my_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;

    /* Determine how much space is left in our fixed-size buffer */
    to_copy = min(count, (size_t)(sizeof(kernel_buffer) - *offs));

    if (to_copy <= 0) {
        pr_warn("test_cdev: No space left in buffer for writing\n");
        return -ENOMEM;
    }

    /* Transfer data from user space to our kernel-space buffer */
    not_copied = copy_from_user(kernel_buffer + *offs, user_buffer, to_copy);
    delta = to_copy - not_copied;

    /* Update offset after a successful write */
    *offs += delta;

    printk(KERN_INFO "test_cdev: Wrote %d bytes to device\n", delta);
    return delta;
}

/* Mapping system calls to our function implementations */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

/**
 * @brief Module Initialization - Manually sets up the character device.
 */
static int __init test_cdev_init(void) {
    int status;

    /* 1. Dynamically allocate a range of device numbers */
    status = alloc_chrdev_region(&device_number, 0, 1, "test_cdev");
    if (status != 0) {
        printk(KERN_ERR "test_cdev: Error allocating device numbers\n");
        return status;
    }

    /* 2. Initialize the character device structure with our file operations */
    cdev_init(&test_cdev, &fops);
    test_cdev.owner = THIS_MODULE;

    /* 3. Add the character device to the system */
    status = cdev_add(&test_cdev, device_number, 1);
    if (status != 0) {
        /* Use goto for efficient error cleanup */
        goto free_device_number;
    }

    /* Successfully registered - Log the major/minor assigned */
    printk(KERN_INFO "test_cdev: Registered with Major: %d, Minor: %d\n", 
           MAJOR(device_number), MINOR(device_number));

    return 0;

free_device_number:
    unregister_chrdev_region(device_number, 1);
    return status;
}

/**
 * @brief Module Exit - Cleans up the character device and reserved numbers.
 */
static void __exit test_cdev_exit(void) {
    /* 1. Remove the character device from the kernel */
    cdev_del(&test_cdev);

    /* 2. Free the reserved device numbers */
    unregister_chrdev_region(device_number, 1);

    /* Log final message using KERN_ALERT to demonstrate another log level */
    printk(KERN_ALERT "test_cdev: Module removed from kernel\n");
}

module_init(test_cdev_init);
module_exit(test_cdev_exit);

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vinoth Kumar K");
MODULE_DESCRIPTION("A manual character device with full file operations");