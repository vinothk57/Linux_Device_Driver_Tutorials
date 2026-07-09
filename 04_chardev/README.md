# 04 — Manual Character Device Kernel Module

This kernel module, implements a character device using **manual registration** (separating device number allocation from device initialization). It supports full file operations—**open, release, read, and write**

## 1. Compilation
To compile the driver, use the provided Makefile. This will generate the kernel object file named `test_cdev.ko`.

```bash
make
```

## 2. Loading the Module
Insert the module into the kernel using the `insmod` command. This triggers the initialization function that manually allocates device regions and initializes the character device structure.

```bash
sudo insmod test_cdev.ko
```

## 3. Finding the Assigned Device Numbers
Since the module uses `alloc_chrdev_region` for dynamic allocation, the kernel will assign the next available major number. To find the **Major** and **Minor** numbers assigned to `test_cdev`, check the kernel logs:

```bash
dmesg | tail
```

Look for a message like: `test_cdev: Registered with Major: 504, Minor: 0`. You can also verify the registration in the system device list:

```bash
cat /proc/devices | grep test_cdev
```

## 4. Creating the Device Node
Manual registration does not automatically create a file in `/dev`. You must use the `mknod` command to create the device node, linking it to the Major and Minor numbers found in the previous step.

```bash
# Syntax: sudo mknod <path> <type: c for character> <major> <minor>
sudo mknod /dev/test_cdev c <MAJOR_NUMBER> 0
```

## 5. Interacting with the Device
You can interact with the device using standard Linux commands which trigger the module's internal callbacks.

**Writing to the device (Triggers `my_write`):**
```bash
echo "Hello Kernel" > /dev/test_cdev
```

**Reading from the device (Triggers `my_read`):**
```bash
cat /dev/test_cdev
```
*Note: The read function is implemented to transfer data from the kernel's internal buffer back to user space using `copy_to_user`.*

## 6. Cleanup
To properly remove the device node and the kernel module:

```bash
sudo rm /dev/test_cdev
sudo rmmod test_cdev
```

Upon removal, the module executes `cdev_del` and `unregister_chrdev_region` to release the reserved device numbers and clean up kernel structures.