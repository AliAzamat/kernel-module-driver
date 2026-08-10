#include <linux/module.h>   // core module macros
#include <linux/kernel.h>   // KERN_INFO, printk
#include <linux/init.h>     // __init, __exit

// Runs when the module is loaded (insmod). __init lets the kernel free this
// code after init to save memory.
static int __init dpu_hello_init(void) {
    printk(KERN_INFO "dpu_hello: loaded\n");
    return 0;               // 0 = success; non-zero aborts the load
}

// Runs when the module is removed (rmmod).
static void __exit dpu_hello_exit(void) {
    printk(KERN_INFO "dpu_hello: unloaded\n");
}

module_init(dpu_hello_init);
module_exit(dpu_hello_exit);

MODULE_LICENSE("GPL");      // non-GPL taints the kernel and hides some symbols
MODULE_AUTHOR("DPU Journey");
MODULE_DESCRIPTION("Minimal loadable kernel module");
