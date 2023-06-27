#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>

static int __init gpio_driver_init(void)
{
  pr_info("gpio driver initialized");
  return 0;
}

static void __exit gpio_driver_exit(void)
{
  pr_info("gpio driver exited");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bradford-hamilton");
MODULE_DESCRIPTION("basic gpio driver for raspberry pi");
MODULE_VERSION("1.0");

// https://github.com/bradford-hamilton/rpi-gpio-driver.git