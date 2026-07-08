# 02 — GPIO Control Kernel Module

Drive an output GPIO and read an input GPIO from a loadable kernel module using the descriptor-based GPIO API, without device tree configuration.

For prerequisites and generic build/load steps, see the [repository README](../README.md).

## Folder contents

| File         | Description |
|--------------|-------------|
| `gpioctrl.c` | Kernel module source — one output line (e.g. LED) and one input line (e.g. button). |
| `Makefile`   | Out-of-tree build file. Produces `gpioctrl.ko`. |
| `README.md`  | This file. |

After a successful build, the directory will also contain `gpioctrl.ko` and kbuild intermediates (removed by `make clean`).

## Requirements

- A board running Linux with **GPIOLIB** and at least one **gpiochip** exposed to the kernel.
- Two free GPIO lines wired to your circuit (defaults in `gpioctrl.c`: line 21 = output, line 20 = input).
- Kernel headers for the running kernel on the target device.

This example does **not** run meaningfully on most x86 PCs, which lack general-purpose GPIO lines. Use an embedded board (SBC, MCU module, industrial PC with GPIO, etc.).

## Caution: compiling vs loading on a desktop PC

| Step | On a typical PC (no external GPIO board) |
|------|------------------------------------------|
| `make` | **Safe.** Verifies the code builds against your kernel headers. Recommended for development. |
| `sudo insmod gpioctrl.ko` | **Usually harmless, but not useful.** The module looks up global GPIO lines `IO_LED` and `IO_BUTTON` (defaults 21 and 20). On most desktops those lines do not exist — `gpio_to_desc()` fails, module init returns an error, and `insmod` refuses to load the module. Nothing stays loaded and no LED/button circuit is involved. |

**When it could be a problem:** Some machines *do* expose platform GPIO (for example an on-board controller with a low line base). If lines 20 and 21 exist and are used by firmware or other hardware, a successful `insmod` could drive or reconfigure real pins. Before loading on any system:

1. Run `gpiodetect` and `gpioinfo` — confirm which chips and line numbers exist.
2. Only load the module on hardware where those lines are wired for your LED/button test **or** change `IO_LED`, `IO_BUTTON`, and `IO_OFFSET` in `gpioctrl.c` to match your board.

**Practical advice:** Use your PC to **compile** only; run **`insmod` / `rmmod` on the target board** with the correct wiring and line numbers.

## Hardware setup

Example wiring (adapt pins to your board):

| GPIO line | Direction | Typical use |
|-----------|-----------|-------------|
| `IO_LED` (default 21) | Output | LED with current-limiting resistor to GND |
| `IO_BUTTON` (default 20) | Input | Push button to GND (board pull-up) |

Use your board’s schematic or pinmux documentation to pick valid line numbers, then update `IO_LED`, `IO_BUTTON`, and `IO_OFFSET` at the top of `gpioctrl.c`.

## Finding GPIO line numbers

GPIO lines are grouped into **gpiochips**. List chips and line counts:

```bash
gpiodetect
```

Inspect a chip (name, line labels, base offset):

```bash
gpioinfo gpiochip0
# or, on older systems:
cat /sys/class/gpio/gpiochip0/label
cat /sys/class/gpio/gpiochip0/base
cat /sys/class/gpio/gpiochip0/ngpio
```

The value passed to `gpio_to_desc()` is a **global line number** (chip base + offset). If your chip has `base 0`, the line offset within the chip equals the global number. Otherwise set `IO_OFFSET` to the chip base, or change `IO_LED` / `IO_BUTTON` to the full global numbers.

## Makefile

```makefile
obj-m += gpioctrl.o
```

That produces `gpioctrl.ko` from `gpioctrl.c`. The `all` and `clean` targets follow the shared pattern in the [repository README](../README.md#building-a-module).

## Build, load, and test

On the target board:

```bash
make
sudo insmod gpioctrl.ko
lsmod | grep gpioctrl
dmesg | tail
sudo rmmod gpioctrl
make clean
```

Sign the module first if Secure Boot applies — see the [repository README](../README.md#secure-boot-mok-key-pair-enrollment-and-module-signing).

## Expected output

On load (exact message depends on wiring and idle button state):

```text
gpioctrl: input line 20 reads 1
```

On unload:

```text
gpioctrl: output off, module unloaded
```

The output line should be **active** while the module is loaded and **inactive** (low) after `rmmod`.

## What this module does (and does not do)

- **Does:** Uses `gpio_to_desc()`, `gpiod_direction_output()`, `gpiod_direction_input()`, `gpiod_set_value()`, and `gpiod_get_value()` without device tree.
- **Does not:** Handle GPIO interrupts, support multiple boards out of the box, or use the preferred `devm_gpiod_get()` + device-tree flow (covered in later tutorials).

Further reading: [GPIO Descriptor Consumer Interface](https://docs.kernel.org/driver-api/gpio/consumer.html) (kernel documentation).
