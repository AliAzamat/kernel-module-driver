# dpu0 — a Linux Character Device Driver

**A loadable Linux kernel module that exposes `/dev/dpu0`: blocking reads that sleep until data arrives, mutex-protected writes, an `ioctl` control interface, automatic device-node creation through the driver model, and a user-space test that drives it through real syscalls.**

This project crosses the user/kernel boundary from the kernel side. It starts with a minimal module, then builds a character driver that behaves like a small register-style device: user space writes a message, queries its length with an `ioctl`, reads it back, and clears it.

## The driver

`dpu_chardev.c` implements the full `file_operations` set:

| Operation | Behavior |
|---|---|
| `write` | Copies up to 256 bytes from user space with `copy_from_user` under a mutex, then wakes sleeping readers |
| `read` | Sleeps on a wait queue until the buffer has data (`wait_event_interruptible`, so Ctrl-C works), then copies out with `copy_to_user`, honoring the file offset |
| `ioctl DPU_GET_LEN` | `_IOR('D', 1, int)` — returns the stored length |
| `ioctl DPU_CLEAR` | `_IO('D', 2)` — empties the buffer |
| unknown `ioctl` | Returns `-ENOTTY` |

Other details worth noting:

- **Driver-model registration.** `class_create` and `device_create` register `/sys/class/dpu/dpu0`, so udev creates and removes `/dev/dpu0` automatically — no `mknod`.
- **Clean failure unwinding.** Each init step that fails undoes the steps before it, and `PTR_ERR` codes propagate to `insmod`.
- **Single exit path for locks.** Read, write, and ioctl release the mutex through one `out:` label so an error can't return while holding it.

Two smaller modules show the building blocks: `dpu_hello.c` (load/unload with `printk`) and `dpu_alloc.c` (kernel heap allocation with `kzalloc`/`kfree` and `GFP_KERNEL` constraints).

## Building and testing

Requires the headers for the running kernel (e.g. `linux-headers-$(uname -r)`) and root to load modules. `class_create` is called with the single-argument signature introduced in Linux 6.4, so it needs 6.4 or newer.

The Makefile currently builds only `dpu_hello`. To build the character driver, add it to `obj-m`:

```make
obj-m += dpu_hello.o dpu_chardev.o
```

Then:

```bash
make                                  # builds the .ko files via Kbuild
sudo insmod dpu_chardev.ko
ls -l /dev/dpu0
dmesg | tail -1                       # dpu0: ready at /dev/dpu0, major=...

gcc -Wall -Wextra -o dpu_test dpu_test.c
sudo ./dpu_test
```

Expected test output, per `dpu_test.c`:

```text
DPU_GET_LEN -> 7
read -> "link up"
after CLEAR, len = 0
```

```bash
sudo rmmod dpu_chardev
```

[BUILD.md](BUILD.md) has the same steps (it assumes the Makefile change above); [RUNBOOK.md](RUNBOOK.md) covers diagnosing "Invalid module format", a missing `/dev` node, `rmmod` busy errors, hung reads, and lock-related freezes.

## Files

| File | Purpose |
|---|---|
| `dpu_chardev.c` | The character driver |
| `dpu_hello.c` | Minimal loadable module |
| `dpu_alloc.c`, `dpu_state.h` | Kernel heap allocation of per-device state |
| `dpu_test.c` | User-space exerciser: write → ioctl → read → clear |
| `Makefile` | Kbuild wrapper |
