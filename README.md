# Writing a Linux Kernel Module and Char Driver

An intermediate project that crosses the user/kernel boundary from the other side. You write a loadable kernel module, then grow it into a character device driver that user space talks to through a /dev node — implementing open, read, write, and an ioctl for register-style control. You learn the kernel build system (Kbuild), copy_to_user/copy_from_user, and why a bug here can take down the whole machine.

Built step-by-step with [KhwajaLabs Build](https://khwajalabs.com).

## Stack
- C
- Linux Kernel
- Kernel Modules
- Char Devices
- ioctl
- Kbuild
