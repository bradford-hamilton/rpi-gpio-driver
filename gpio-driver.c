#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <asm/io.h>

// Define different proc ops structures depending on kernel version
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

// Procfs macros
#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "gpiobuf"

// Register macros
#define GPIO_REGISTER_BASE 0xfe200000
#define GPIO_SET_REG *(gpio_addr+7)
#define GPIO_CLR_REG *(gpio_addr+10)
#define FSEL_REG(pin) *(gpio_addr + (pin/10))

// Global variables
static struct proc_dir_entry* gpio_proc = NULL;
static volatile unsigned int* gpio_addr = NULL;
static char gpio_proc_buf[PROCFS_MAX_SIZE];

// Sets given gpio pin to input/output through the appropriate function select register
static void set_as_input(unsigned int pin) { FSEL_REG(pin) &= ~(7 << ((pin % 10) * 3)); }
static void set_as_output(unsigned int pin) { FSEL_REG(pin) |= (1 << ((pin % 10) * 3)); }

static void write_pin(unsigned int pin, unsigned int val)
{
  if (val) {
    GPIO_SET_REG = 1 << pin;
  } else {
    GPIO_CLR_REG = 1 << pin;
  }
}

static void gpio_pin_on(unsigned int pin)
{
  set_as_output(pin);
  write_pin(pin, 1);
}

static void gpio_pin_off(unsigned int pin)
{
  set_as_output(pin);
  write_pin(pin, 0);
}

static ssize_t gpio_proc_read(struct file* filep, char __user* user_buf, size_t buf_len, loff_t* offset)
{
  copy_to_user(user_buf, "Successfully read from gpio driver\n", 31);
  return 31;
}

// 17, 27, 22, 23, 24, 25, 5, 6

static ssize_t gpio_proc_write(struct file* filep, const char __user* user_buf, size_t buf_len, loff_t* offset)
{
  memset(gpio_proc_buf, 0x0, sizeof(gpio_proc_buf));

  if (buf_len > PROCFS_MAX_SIZE) {
    buf_len = PROCFS_MAX_SIZE;
  }

  copy_from_user(gpio_proc_buf, user_buf, buf_len);

  // int pin;
  // int io;

  // int num_parsed = sscanf(gpio_proc_buf, "%d,%d", &pin, &io);
  // if (num_parsed != 2) {
  //   pr_alert("error: invalid data format submitted\n");
  //   return buf_len;
  // }
  // if (pin < 2 || pin > 27) {
  //   pr_alert("error: invalid GPIO pin number\n");
  //   return buf_len;
  // }
  // if (io != 0 && io != 1) {
  //   pr_alert("error: pin io must be 0 or 1 only\n");
  //   return buf_len;
  // }

  // pr_info("You wrote '%d,%d' to gpio driver\n", pin, io);

  // if (io == 0) {
  //   gpio_pin_off(pin);
  // } else if (io == 1) {
  //   gpio_pin_on(pin);
  // } else {
  //   pr_alert("You found an edge case, pin: %d and io: %d\n", pin, io);
  // }

  gpio_pin_on(17);
  msleep(500);
  gpio_pin_off(17);
  msleep(500);

  gpio_pin_on(27);
  msleep(500);
  gpio_pin_off(27);
  msleep(500);

  gpio_pin_on(22);
  msleep(500);
  gpio_pin_off(22);
  msleep(500);
  
  gpio_pin_on(23);
  msleep(500);
  gpio_pin_off(23);
  msleep(500);

  gpio_pin_on(24);
  msleep(500);
  gpio_pin_off(24);
  msleep(500);

  gpio_pin_on(25);
  msleep(500);
  gpio_pin_off(25);
  msleep(500);

  gpio_pin_on(5);
  msleep(500);
  gpio_pin_off(5);
  msleep(500);

  gpio_pin_on(6);
  msleep(500);
  gpio_pin_off(6);
  msleep(500);

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
  gpio_addr = (volatile unsigned int*)ioremap(GPIO_REGISTER_BASE, 4096);
  if (gpio_addr == NULL) {
    pr_alert("error: failed to remap gpio register base\n");
    return -1;
  }
  pr_info("successfully mapped GPIO memory\n");

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
  iounmap(gpio_addr);
  proc_remove(gpio_proc);
  pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bradford-hamilton");
MODULE_DESCRIPTION("basic gpio driver for raspberry pi");
MODULE_VERSION("1.0");
