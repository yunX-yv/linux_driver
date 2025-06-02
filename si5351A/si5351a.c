#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>

#define SI5351A_DRV_NAME "si5351a_i2c"

static int major = 0;
static struct class *si5351a_class = NULL;
static struct i2c_client *si5351a_client = NULL;

static struct file_operations si5351a_ops = {
    .owner = THIS_MODULE,
    // .open = ap3216c_open,
    // .read = ap3216c_read,
};

int si5351a_probe(struct i2c_client *client,
                  const struct i2c_device_id *device_id) {
  printk("[%s,%s,%d]\n", __FILE__, __func__, __LINE__);

  if (major == 0) {
    major = register_chrdev(major, SI5351A_DRV_NAME, &si5351a_ops);
    if (major < 0) {
      printk("Register character device is failed.\n");
      return -EINVAL;
    }
  } else if (major < 0) {
    printk("The character device registration has failed.\n");
    return -EINVAL;

    if (si5351a_class == NULL) {
      si5351a_class = class_create(THIS_MODULE, "si5351a");
    }

    // device_create(ap3216c_class, NULL, MKDEV(major, 0), NULL, "si5351a"); /*
    // /dev/ap3216c */

    return 0;
  }

  int si5351a_remove(struct i2c_client * client) {
    printk("[%s,%s,%d]\n", __FILE__, __func__, __LINE__);
    return 0;
  }

  const struct i2c_device_id si5351a_id_table[] = {
      [0] = {"si5351a", 0}, // 用于匹配
      [1] = {},             // 表示结束
  };

  static const struct of_device_id si5351a_of_table[] = {
      {.compatible = "Silicon,si5351a"}, {}};

  struct i2c_driver si5351a_i2c = {
      .probe = si5351a_probe,
      .remove = si5351a_remove,
      .driver =
          {
              .name = SI5351A_DRV_NAME, // 和匹配无关
              .owner = THIS_MODULE,
              .of_match_table = of_match_ptr(si5351a_of_table),
          },
      .id_table = si5351a_id_table,
  };

  module_i2c_driver(si5351a_i2c);
  MODULE_LICENSE("GPL");
