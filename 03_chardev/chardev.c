#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h> /* Required for character device functions */
#include <linux/uaccess.h> /* Required for copy_to_user */

/* Global variables */
static int major; /* Stores the major device number */

/**
 * @brief This function is called when the device file is read
 */
static ssize_t custom_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs) {
    char *message = "Vinoth Kumar\n";
    int message_len = 13; // Length of "Vinoth Kumar\n"
    int to_copy, not_copied, delta;

    /* Check if the user has already read the message (offset management) */
    /* Returning 0 tells the user-space program (like cat) that there is no more data */
    if (*offs >= message_len) {
        return 0;
    }

    /* Calculate how many bytes to copy */
    to_copy = min(count, (size_t)(message_len - *offs));

    /* copy_to_user is the standard way to send data to the user-space buffer */
    /* It returns the number of bytes that could NOT be copied */
    not_copied = copy_to_user(user_buffer, message + *offs, to_copy);

    /* Calculate how many bytes were actually copied */
    delta = to_copy - not_copied;

    /* Update the file offset so the next read knows where we left off */
    *offs += delta;

    /* Log the operation using a low-priority level */
    printk(KERN_DEBUG "test_cdev: Sent %d bytes to user space\n", delta);

    return delta; // Return the number of bytes successfully read
}

/* File operations structure: links system calls to module functions */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = custom_read, /* Overload the read function */
};

/**
 * @brief This function is called when the module is loaded into the kernel
 */
static int __init custom_module_init(void) {
    /* Register the character device. Passing 0 allows dynamic major number allocation */
    major = register_chrdev(0, "test_cdev", &fops);

    if (major < 0) {
        /* Use KERN_ERR for error conditions */
        printk(KERN_ERR "test_cdev: Could not register character device\n");
        return major;
    }

    /* Demonstrate different log levels */
    printk(KERN_EMERG "Log Level: Emergency\n");
    printk(KERN_ALERT "Log Level: Alert\n");
    printk(KERN_WARNING "Log Level: Warning\n");
    
    /* Using KERN_INFO for general status messages */
    printk(KERN_INFO "test_cdev: Registered with major number %d\n", major);

    return 0;
}

/**
 * @brief This function is called when the module is removed from the kernel
 */
static void __exit custom_module_exit(void) {
    /* Unregister the device and free the major number */
    unregister_chrdev(major, "test_cdev");

    /* Using an alias macro for logging */
    pr_info("test_cdev: Module unloaded successfully\n");
}

module_init(custom_module_init);
module_exit(custom_module_exit);

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vinoth Kumar K");
MODULE_DESCRIPTION("A character device that returns a specific string");