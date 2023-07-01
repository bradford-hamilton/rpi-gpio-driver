#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <asm/io.h>

// For defining different proc ops structures depending on kernel version
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
static int pins[8] = {6, 5, 25, 24, 23, 22, 27, 17};

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

// 17, 27, 22, 23, 24, 25, 5, 6
void set_leds(unsigned int num) {
  int i;
  for (i = 7; i >= 0; i--) {
    unsigned int bit = (num >> i) & 1;
    if (bit == 1) {
      gpio_pin_on(pins[i]);
    }
  }

  ssleep(5);

  for (i = 7; i >= 0; i--) {
    gpio_pin_off(pins[i]);
  }
}

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

  unsigned int num;
  int num_parsed = sscanf(gpio_proc_buf, "%d", &num);

  if (num_parsed != 1) {
    pr_alert("error: invalid data format submitted\n");
    return buf_len;
  }

  if (num < 0 || num > 255) {
    pr_alert("error: invalid 8-bit number\n");
    return buf_len;
  }

  set_leds(num);

  pr_info("You wrote '%d' to gpio driver\n", num);

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
MODULE_DESCRIPTION("kernel module for rpi4 GPIO interaction");
MODULE_VERSION("1.0");
