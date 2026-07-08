# 01 — Hello World Kernel Module

A minimal loadable kernel module (LKM) that prints a message when loaded and unloaded. This example introduces the module lifecycle: build, load, inspect, and remove.

For prerequisites and generic build/load steps, see the [repository README](../README.md).

## Folder contents

| File           | Description |
|----------------|-------------|
| `helloworld.c` | Source for the kernel module. Defines `mymodule_init()` and `mymodule_exit()` and registers them with `module_init()` / `module_exit()`. |
| `Makefile`     | Out-of-tree build file. Invokes the running kernel’s build system to compile `helloworld.ko`. |
| `README.md`    | This file. |

After a successful build, the directory will also contain:

| File / artifact | Description |
|-----------------|-------------|
| `helloworld.ko` | The compiled kernel module (load with `insmod` or `modprobe`). |
| `*.o`, `*.mod*`, `Module.symvers`, etc. | Intermediate build files (removed by `make clean`). |

## Makefile

This tutorial’s module object is declared as:

```makefile
obj-m += helloworld.o
```

That produces `helloworld.ko` from `helloworld.c`. The `all` and `clean` targets follow the shared pattern described in the [repository README](../README.md#building-a-module).

## Build, load, and test

From this directory:

```bash
make
sudo insmod helloworld.ko
lsmod | grep helloworld
dmesg | tail
modinfo helloworld.ko
sudo rmmod helloworld
make clean
```

## Expected output

On load:

```text
Hello world
```

On unload:

```text
Goodbye world
```

These lines come from `printk(KERN_INFO, ...)` in `helloworld.c`.

## What this module does (and does not do)

- **Does:** Demonstrates `__init` / `__exit`, `module_init` / `module_exit`, and `MODULE_*` metadata macros.
- **Does not:** Register a character device, touch hardware, or expose a `/dev` node. Those come in later tutorials.
