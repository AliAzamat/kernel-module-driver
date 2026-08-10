# Writing a Linux Kernel Module and Char Driver

A project that crosses the user/kernel boundary from the other side. Wrote a loadable kernel module, then grew it into a character device driver that user space talks to through a /dev node — implementing open, read, write, and an ioctl for register-style control. Utilizes the kernel build system (Kbuild), copy_to_user/copy_from_user.

## Stack
- C
- Linux Kernel
- Kernel Modules
- Char Devices
- ioctl
- Kbuild
