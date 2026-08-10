# Building and testing dpu0

## Build the module
    make                       # produces dpu_chardev.ko

## Load (node is created automatically now)
    sudo insmod dpu_chardev.ko
    ls -l /dev/dpu0            # udev created it — no mknod needed
    dmesg | tail -1           # "dpu0: ready at /dev/dpu0, major=..."

## Build and run the test
    gcc -Wall -Wextra -o dpu_test dpu_test.c
    sudo ./dpu_test           # write -> ioctl -> read -> clear round trip

Expected output:
    DPU_GET_LEN -> 7
    read -> "link up"
    after CLEAR, len = 0

## Unload (no rm needed — udev removes the node)
    sudo rmmod dpu_chardev
