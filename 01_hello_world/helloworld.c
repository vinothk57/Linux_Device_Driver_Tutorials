/*
 * helloworld.c - A minimal Linux kernel module (loadable kernel module / LKM)
 *
 * This is the simplest possible driver: it prints a message when loaded and
 * unloaded. No hardware is involved; it only demonstrates module lifecycle.
 */

#include <linux/module.h>  /* Core module support: module_init, MODULE_* macros */
#include <linux/init.h>      /* __init and __exit — mark init/exit code for memory reclaim */
#include <linux/kernel.h>    /* KERN_INFO and printk() */

/*
 * mymodule_init() - Called once when the module is loaded (insmod/modprobe).
 *
 * Return 0 on success; any non-zero value aborts loading and the module is not kept loaded.
 */
static int __init mymodule_init(void)
{
	/* printk() writes to the kernel log (view with dmesg or journalctl -k) */
	printk(KERN_INFO "Hello world\n");
	return 0;
}

/*
 * mymodule_exit() - Called when the module is unloaded (rmmod).
 *
 * Use this to release resources (memory, IRQs, device registrations, etc.).
 * This example has nothing to clean up.
 */
static void __exit mymodule_exit(void)
{
	printk(KERN_INFO "Goodbye world\n");
}

/* Register entry and exit handlers with the module loader */
module_init(mymodule_init);
module_exit(mymodule_exit);

/* Module metadata — visible in modinfo helloworld.ko */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vinoth Kumar K");
MODULE_DESCRIPTION("Hello World Kernel Module");
