#include <linux/module.h>
#include <linux/slab.h>        // kmalloc, kzalloc, kfree
#include <linux/init.h>
#include "dpu_state.h"

// The kernel heap analog of malloc. There is no libc here.
static struct dpu_state *state;

static int __init dpu_init(void) {
    // kzalloc = kmalloc + zero. GFP_KERNEL: may sleep to reclaim memory, so it
    // is only legal in a context that can sleep (NOT an interrupt handler).
    state = kzalloc(sizeof(*state), GFP_KERNEL);
    if (!state) {
        return -ENOMEM;        // out of kernel memory — abort the load
    }
    state->buf_len = 0;
    printk(KERN_INFO "dpu_alloc: state allocated (%zu bytes)\n", sizeof(*state));
    return 0;
}

static void __exit dpu_exit(void) {
    kfree(state);              // give the kernel heap memory back
    state = NULL;              // no dangling pointer in the module
    printk(KERN_INFO "dpu_alloc: state freed\n");
}

module_init(dpu_init);
module_exit(dpu_exit);
MODULE_LICENSE("GPL");
