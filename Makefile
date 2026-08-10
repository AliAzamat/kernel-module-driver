# A kernel module is built BY the kernel's own build system (Kbuild), not by a
# plain gcc invocation. obj-m tells Kbuild to build dpu_hello.ko from .c.
obj-m += dpu_hello.o

# KDIR points at the running kernel's build tree (its headers + Kbuild).
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
