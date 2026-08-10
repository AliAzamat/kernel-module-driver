#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/wait.h>        // wait queues
#include <linux/sched.h>

#define DEVICE_NAME "dpu0"
#define CLASS_NAME  "dpu"
#define BUF_LEN 256

#define DPU_IOC_MAGIC   'D'
#define DPU_GET_LEN     _IOR(DPU_IOC_MAGIC, 1, int)
#define DPU_CLEAR       _IO(DPU_IOC_MAGIC, 2)

static int major;
static struct class  *dpu_class;
static struct device *dpu_device;
static char kbuf[BUF_LEN];
static size_t kbuf_len;
static DEFINE_MUTEX(dpu_lock);
static DECLARE_WAIT_QUEUE_HEAD(dpu_readq);   // readers sleep here

static ssize_t dpu_read(struct file *f, char __user *ubuf, size_t len, loff_t *off) {
    ssize_t ret;

    // Block until there is data. wait_event_interruptible sleeps the caller and
    // returns when the condition is true — or -ERESTARTSYS if a signal arrives.
    if (wait_event_interruptible(dpu_readq, kbuf_len > 0)) {
        return -ERESTARTSYS;
    }

    mutex_lock(&dpu_lock);
    if (*off >= (loff_t)kbuf_len) { ret = 0; goto out; }
    {
        size_t n = min(len, kbuf_len - (size_t)*off);
        if (copy_to_user(ubuf, kbuf + *off, n)) { ret = -EFAULT; goto out; }
        *off += n;
        ret = (ssize_t)n;
    }
out:
    mutex_unlock(&dpu_lock);
    return ret;
}

static ssize_t dpu_write(struct file *f, const char __user *ubuf, size_t len, loff_t *off) {
    ssize_t ret;
    size_t n = min(len, (size_t)BUF_LEN);
    mutex_lock(&dpu_lock);
    if (copy_from_user(kbuf, ubuf, n)) { ret = -EFAULT; goto out; }
    kbuf_len = n;
    ret = (ssize_t)n;
out:
    mutex_unlock(&dpu_lock);
    if (ret > 0) {
        wake_up_interruptible(&dpu_readq);   // data arrived: wake sleeping readers
    }
    return ret;
}

static long dpu_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    long ret = 0;
    mutex_lock(&dpu_lock);
    switch (cmd) {
    case DPU_GET_LEN: {
        int len = (int)kbuf_len;
        if (copy_to_user((int __user *)arg, &len, sizeof(len))) ret = -EFAULT;
        break;
    }
    case DPU_CLEAR:
        kbuf_len = 0;
        break;
    default:
        ret = -ENOTTY;
    }
    mutex_unlock(&dpu_lock);
    return ret;
}

static int dpu_open(struct inode *inode, struct file *f) { return 0; }
static int dpu_release(struct inode *inode, struct file *f) { return 0; }

static const struct file_operations dpu_fops = {
    .owner          = THIS_MODULE,
    .open           = dpu_open,
    .release        = dpu_release,
    .read           = dpu_read,
    .write          = dpu_write,
    .unlocked_ioctl = dpu_ioctl,
};

static int __init dpu_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &dpu_fops);
    if (major < 0) {
        return major;
    }
    dpu_class = class_create(CLASS_NAME);
    if (IS_ERR(dpu_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(dpu_class);
    }
    dpu_device = device_create(dpu_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(dpu_device)) {
        class_destroy(dpu_class);
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(dpu_device);
    }
    printk(KERN_INFO "dpu0: ready at /dev/%s, major=%d\n", DEVICE_NAME, major);
    return 0;
}

static void __exit dpu_exit(void) {
    device_destroy(dpu_class, MKDEV(major, 0));
    class_destroy(dpu_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "dpu0: removed\n");
}

module_init(dpu_init);
module_exit(dpu_exit);
MODULE_LICENSE("GPL");
