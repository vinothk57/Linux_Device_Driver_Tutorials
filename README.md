# Linux Device Driver Tutorials

Loadable kernel module (LKM) examples developed on Ubuntu 24.04. Each numbered folder is a self-contained tutorial with its own source, Makefile, and README.

## Tutorials

| Folder | Topic |
|--------|-------|
| [01_hello_world](01_hello_world/) | Minimal module — load, print, unload |

## Prerequisites

Install kernel headers and build tools for your **running** kernel (Ubuntu/Debian example):

```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

Every tutorial Makefile uses `/lib/modules/$(uname -r)/build`, so the installed headers must match the kernel you booted. If `make` fails with a missing build directory, install the matching `linux-headers` package and retry.

If Secure Boot is enabled, you also need `mokutil` to enroll a signing key:

```bash
sudo apt install mokutil
```

Check whether Secure Boot is on:

```bash
mokutil --sb-state
```

If it reports **SecureBoot enabled**, unsigned modules will be rejected at load time (for example, `insmod` fails with *Required key not available*). Generate and enroll a Machine Owner Key (MOK) pair as described below, then sign each `.ko` before loading.

## Secure Boot: MOK key pair, enrollment, and module signing

These steps create a personal key pair, enroll the public key in firmware via MOK Manager, and use the private key to sign built modules. You only need to enroll once; re-sign after each rebuild.

### 1. Generate a MOK key pair

Run from a directory where you keep signing material (not inside a tutorial folder you commit to git). **Keep `MOK.priv` secret** — anyone with it can sign kernel modules as you.

```bash
openssl req -new -x509 -newkey rsa:2048 \
  -keyout MOK.priv \
  -outform DER -out MOK.der \
  -nodes -days 36500 \
  -subj "/CN=Local Kernel Module Signing Key/"
```

This produces:

| File       | Purpose |
|------------|---------|
| `MOK.priv` | Private key — used to sign `.ko` files |
| `MOK.der`  | Public certificate — enrolled in MOK and paired with the private key |

### 2. Enroll the public key (one-time)

Import the public key into the MOK list. `mokutil` asks for a **one-time enrollment password** (used only at the next reboot in MOK Manager, not for signing):

```bash
sudo mokutil --import MOK.der
```

Reboot. During startup, the **MOK Management** (blue) screen appears:

1. Choose **Enroll MOK**
2. **Continue** → **Yes**
3. Enter the enrollment password you set above
4. **Reboot**

After boot, confirm the key is enrolled:

```bash
mokutil --list-enrolled | grep -i "Local Kernel Module Signing Key"
mokutil --test-key MOK.der
```

The test command should report that the key is enrolled.

### 3. Sign a built module

Build the module as usual (`make`), then sign the `.ko` with the kernel’s `sign-file` script (from the same headers package used to build):

```bash
/lib/modules/$(uname -r)/build/scripts/sign-file sha256 \
  /path/to/MOK.priv /path/to/MOK.der modulename.ko
```

Example for `01_hello_world` (adjust paths to where you stored the keys):

```bash
cd 01_hello_world
make
/lib/modules/$(uname -r)/build/scripts/sign-file sha256 \
  ~/keys/MOK.priv ~/keys/MOK.der helloworld.ko
```

Every rebuild produces an unsigned `.ko` again — run `sign-file` after each `make` before loading.

### 4. Load the signed module

```bash
sudo insmod modulename.ko
```

If loading still fails with a signature error, verify Secure Boot state, that the key is enrolled (`mokutil --test-key`), and that you signed the same `.ko` you are loading.

## Building a module

Each tutorial lives in its own directory and uses an out-of-tree **kbuild** Makefile. From that directory:

```bash
make          # build the .ko module
make clean    # remove the .ko and generated build files
```

Typical Makefile layout:

```makefile
obj-m += modulename.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

- `obj-m += foo.o` — tell kbuild to produce a loadable module `foo.ko` from `foo.c`.
- `-C /lib/modules/.../build` — use the installed kernel build tree.
- `M=$(PWD)` — treat the current directory as the external module source path.
- `modules` / `clean` — kbuild goals to build or clean the module.

After a successful build you will see `modulename.ko` plus intermediate files (`*.o`, `*.mod*`, `Module.symvers`, etc.). `make clean` removes them.

## Loading, inspecting, and unloading

Most steps require root:

```bash
sudo insmod modulename.ko    # load the module
lsmod | grep modulename      # confirm it is loaded
dmesg | tail                 # read recent kernel log messages
sudo journalctl -k --since "1 min ago"   # alternative on systemd systems
modinfo modulename.ko        # show module metadata
sudo rmmod modulename        # unload (name without .ko)
```

`printk()` output from a module goes to the kernel log (`dmesg` / `journalctl -k`), not necessarily to your terminal.
