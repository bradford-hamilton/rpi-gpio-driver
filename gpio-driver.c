#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "gpiobuf"

static struct proc_dir_entry* gpio_proc = NULL;
static char gpio_proc_buf[PROCFS_MAX_SIZE];

static ssize_t gpio_proc_read(struct file* filep, char __user* user_buf, size_t buf_len, loff_t* offset)
{
  copy_to_user(user_buf, "Successfully read from gpio driver\n", 31);
  return 31;
}

static ssize_t gpio_proc_write(struct file* filep, const char __user* user_buf, size_t buf_len, loff_t* offset)
{
  memset(gpio_proc_buf, 0x0, sizeof(gpio_proc_buf));

  if (buf_len > PROCFS_MAX_SIZE) {
    buf_len = PROCFS_MAX_SIZE;
  }

  copy_from_user(gpio_proc_buf, user_buf, buf_len);

  pr_info("You wrote '%s' to gpio driver\n", gpio_proc_buf);

  return buf_len;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
  .proc_read = gpio_proc_read,
  .proc_write = gpio_proc_write,
};
#else
static const struct file_operations proc_file_fops = {
  .read = gpio_proc_read,
  .write = gpio_proc_write,
};
#endif

static int __init gpio_driver_init(void)
{
  gpio_proc = proc_create(PROCFS_NAME, 0666, NULL, &proc_file_fops);
  if (gpio_proc == NULL) {
    proc_remove(gpio_proc);
    pr_alert("error: could not initialize /proc/%s\n", PROCFS_NAME);
    return -1;
  }

  pr_info("/proc/%s intialized\n", PROCFS_NAME);

  return 0;
}

static void __exit gpio_driver_exit(void)
{
  proc_remove(gpio_proc);
  pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bradford-hamilton");
MODULE_DESCRIPTION("basic gpio driver for raspberry pi");
MODULE_VERSION("1.0");
