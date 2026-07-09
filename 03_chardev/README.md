
# 03 — Character device Kernel Module
This kernel module implements a basic character device that returns the string "Vinoth Kumar" when read. It also demonstrates the use of various Linux kernel log levels to categorize system messages.

## 1. Compilation
To compile the driver, use your Makefile. This will generate the kernel object file named chardev.ko

```bash
make
```

## 2. Loading the Module
Insert the module into the kernel using the insmod command:

```bash
sudo insmod chardev.ko
```

## 3. Finding the Dynamic Major Number
The module uses dynamic allocation by passing 0 to the register_chrdev function, which searches for a free major device number. To find the number assigned to your device, check the kernel logs:

```bash
dmesg | tail
```

Look for a message like: test_cdev: Registered with major number 504. Note this number for the next step.

## 4. Creating the Device Node
You must create a device file in /dev to act as an interface between user space and your driver. Use the mknod command with the major number you found above:

```bash
# Syntax: sudo mknod <path> <type: c for character> <major> <minor>
sudo mknod /dev/chardev c <MAJOR_NUMBER> 0
```

## 5. Interacting with the Device
To read the message "Vinoth Kumar" from the kernel module, use the cat command. This triggers the read function defined in the module's file_operations struct.

```bash
cat /dev/chardev

# Output: Vinoth Kumar
```

## 6. Observing Log Levels
The module uses different printk log levels (ranging from 0 for Emergency to 7 for Debug) to structure its output. You can view these messages and filter them using dmesg:

View all logs: 
```bash
dmesg
```

Filter by level: Use dmesg -l <level_number> to see specific priorities (e.g., dmesg -l 1 for alerts).

The levels demonstrated include:
```bash
KERN_EMERG (0): System is unusable.
KERN_ALERT (1): Action must be taken immediately.
KERN_WARNING (4): Warning conditions.
KERN_INFO (6): Informational messages (e.g., successful registration).
KERN_DEBUG (7): Debug-level messages (e.g., when read is called).
```

## 7. Cleanup
To remove the device and the module from your system:

```bash
sudo rm /dev/chardev
sudo rmmod chardev
```

The unregister_chrdev function will be called during removal to free the allocated device numbers