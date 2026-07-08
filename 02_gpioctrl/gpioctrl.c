/*
 * gpioctrl.c - GPIO input/output from a kernel module (no device tree)
 *
 * Uses the descriptor-based GPIO consumer API to drive an output line (e.g. an
 * LED) and read an input line (e.g. a push button).
 *
 * Requires a platform with kernel GPIO support (GPIOLIB). See README.md for
 * hardware and load/unload steps.
 */

#include <linux/module.h>        /* module_init, MODULE_* macros */
#include <linux/init.h>          /* __init, __exit */
#include <linux/kernel.h>        /* KERN_INFO, printk() */
#include <linux/gpio/consumer.h> /* struct gpio_desc, gpiod_*(), gpio_to_desc() */

/*
 * Line numbers within the gpiochip (not physical header pin numbers).
 * Change these to match your board schematic / gpiod line index.
 */
#define IO_LED     21
#define IO_BUTTON  20

/*
 * IO_OFFSET — gpiochip base (global line number of chip-local line 0).
 *
 * gpio_to_desc() takes a *global* line number:
 *
 *   global_line = IO_OFFSET + chip_local_line
 *
 * Read the base from sysfs, e.g.:
 *   cat /sys/class/gpio/gpiochip512/base
 *   → 512
 *
 * Example A — chip-local line 21 on gpiochip512 (base 512):
 *   IO_OFFSET = 512
 *   IO_LED    = 21
 *   gpio_to_desc(512 + 21)  →  global line 533
 *
 * Example B — chip-local line 21 on gpiochip0 (base 0):
 *   IO_OFFSET = 0
 *   IO_LED    = 21
 *   gpio_to_desc(0 + 21)    →  global line 21
 */
#define IO_OFFSET  0

static struct gpio_desc *led;
static struct gpio_desc *button;

/*
 * mymodule_init() - Configure GPIO lines and demonstrate read/write.
 */
static int __init mymodule_init(void)
{
	int status;

	/*
	 * gpio_to_desc() — look up a kernel GPIO descriptor by global line number.
	 * Returns NULL if the line does not exist. Does not configure direction yet.
	 */
	led = gpio_to_desc(IO_LED + IO_OFFSET);
	if (!led) {
		printk(KERN_ERR "gpioctrl: failed to get GPIO line %d\n", IO_LED);
		return -ENODEV;
	}

	button = gpio_to_desc(IO_BUTTON + IO_OFFSET);
	if (!button) {
		printk(KERN_ERR "gpioctrl: failed to get GPIO line %d\n", IO_BUTTON);
		return -ENODEV;
	}

	/*
	 * gpiod_direction_output() — configure the line as an output.
	 * Second argument is the initial logical level (0 = low, 1 = high).
	 */
	status = gpiod_direction_output(led, 0);
	if (status) {
		printk(KERN_ERR "gpioctrl: failed to set line %d as output\n", IO_LED);
		return status;
	}

	/* gpiod_direction_input() — configure the line as an input. */
	status = gpiod_direction_input(button);
	if (status) {
		printk(KERN_ERR "gpioctrl: failed to set line %d as input\n", IO_BUTTON);
		return status;
	}

	/* gpiod_set_value() — drive an output line high (1) or low (0). */
	gpiod_set_value(led, 1);

	/*
	 * gpiod_get_value() — read the current logical level of a line (0 or 1).
	 * Works for inputs; for outputs, returns the driven value.
	 */
	printk(KERN_INFO "gpioctrl: input line %d reads %d\n",
	       IO_BUTTON, gpiod_get_value(button));

	return 0;
}

/*
 * mymodule_exit() - Drive the output line low before the module is unloaded.
 */
static void __exit mymodule_exit(void)
{
	gpiod_set_value(led, 0);
	printk(KERN_INFO "gpioctrl: output off, module unloaded\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vinoth Kumar K");
MODULE_DESCRIPTION("GPIO output and input example without device tree");
