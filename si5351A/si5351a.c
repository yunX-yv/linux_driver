#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#define SI5351A_CLASS_NAME "si5351x_Out"
#define SI5351A_DEV_NAME "si5351x"

struct output_clock_device {
  struct device *dev;
  struct clk **clks;
  struct device **devs;
  struct cdev *cdev;
  int num_clks;
};
static struct output_clock_device *clk_dev;

ssize_t led_read(struct file *file, char __user *ubuf, size_t size,
                 loff_t *loft) {
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  return 0;
}
ssize_t led_write(struct file *file, const char __user *ubuf, size_t size,
                  loff_t *loft) {
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);

  switch ((int)file->private_data) {
  case 0:
    printk("mydev0\n");
    break;
  }
  return size;
}
int led_open(struct inode *inode, struct file *file) {
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  file->private_data = (void *)MINOR(inode->i_rdev); // 储存打开的设备的子设备号
  return 0;
}
int led_release(struct inode *inode, struct file *file) {
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  return 0;
}

struct file_operations si5351a_fops = {
    .open = led_open,
    .release = led_release,
    .write = led_write,
    .read = led_read,
};

int si5351a_user_probe(struct platform_device *pdev) {
  struct clk *clk;
  const char *const *clk_names = NULL;
  int i, count;
  struct cdev *cdev;
  int ret;
  dev_t dev_c;
  struct class *cls;
  int major; // 获取申请到的主设备号
  int minor; // 获取申请到的次设备号

  clk_dev = devm_kzalloc(&pdev->dev, sizeof(*clk_dev), GFP_KERNEL);
  if (!clk_dev)
    return -ENOMEM;

  clk_dev->dev = &pdev->dev;

  /* first, get clocks from device tree */
  count =
      of_count_phandle_with_args(pdev->dev.of_node, "clocks", "#clock-cells");
  if (count <= 0) {
    dev_err(&pdev->dev, "No clocks defined in device tree\n");
    ret = -ENOMEM;
    goto err_free;
  }
  clk_dev->num_clks = count;

  clk_names = of_get_property(pdev->dev.of_node, "clock-names", NULL);
  if (!clk_names) {
    pr_err("No clock-names property found\n");
    ret = -ENOMEM;
    goto err_free;
  }

  for (i = 0; i < clk_dev->num_clks; i++) {
    clk = devm_clk_get(&pdev->dev, clk_names[i]);
    if (IS_ERR(clk)) {
      dev_err(&pdev->dev, "Failed to get clock %d\n", i);
      ret = -ENOMEM;
      goto err_free;
    }
    dev_info(&pdev->dev, "Clock %d: %s\n", i, clk_names[i]);
    clk_dev->clks[i] = clk;
    clk_unprepare(clk);
  }

  cdev = cdev_alloc(); // 字符设备结构体申请内存
  if (cdev == NULL) {
    printk("[%s,%s,%d]cdev_alloc err\n", __FILE__, __func__, __LINE__);
    ret = -ENOMEM;
    goto err_free;
  }
  cdev_init(cdev, &si5351a_fops); // 初始化字符设备结构体

  ret = alloc_chrdev_region(&dev_c, 0, clk_dev->num_clks, SI5351A_DEV_NAME);
  if (ret) {
    printk("[%s,%s,%d]alloc_chrdev_region err\n", __FILE__, __func__, __LINE__);
    goto err_free;
  }
  major = MAJOR(dev_c); // 获取申请到的主设备号
  minor = MINOR(dev_c); // 获取申请到的次设备号

  ret = cdev_add(cdev, MKDEV(major, minor),
                 clk_dev->num_clks); // 按设备号注册设备
  if (ret) {
    printk("[%s,%s,%d]cedv_add err\n", __FILE__, __func__, __LINE__);
    goto err_free;
  }

  // 2.注册设备节点
  cls =
      class_create(THIS_MODULE, SI5351A_CLASS_NAME); // 提交设备节点目录
  if (IS_ERR(cls)) {
    printk("[%s,%s,%d]class_create err\n", __FILE__, __func__, __LINE__);
    ret = PTR_ERR(cls);
    goto err_free;
  }

  for (i = 0; i < clk_dev->num_clks; i++) {
    // 提交设备节点文件名
    struct device *dev =
        device_create(cls, NULL, MKDEV(major, i), NULL, "my_led%d", i);
    if (IS_ERR(dev)) {
      printk("[%s,%s,%d]device_create err\n", __FILE__, __func__, __LINE__);
      ret = PTR_ERR(dev);
      goto err_free;
    }
    clk_dev->devs[i] = dev;
  }

  return 0;

err_free:
  for (i = 0; i < clk_dev->num_clks; i++) {
    if (clk_dev->clks[i]) {
      clk_unprepare(clk_dev->clks[i]);
      devm_clk_put(&pdev->dev, clk_dev->clks[i]);
    }
  }
  devm_kfree(&pdev->dev, clk_dev->clks);
  return ret;
}

int si5351a_user_remove(struct platform_device *pdev) {
  int i;
  for (i = 0; i < clk_dev->num_clks; i++) {
    if (clk_dev->clks[i])
      devm_clk_put(&pdev->dev, clk_dev->clks[i]);
  }
  devm_kfree(&pdev->dev, clk_dev->clks);
  return 0;
}

static const struct platform_device_id si5351a_id_table[] = {
    [0] = {"si5351a-usr", 0}, // 用于匹配
    [1] = {},                 // 表示结束
};

static const struct of_device_id si5351a_of_table[] = {
    {.compatible = "Silicon,si5351a-usr"}, {}};

static struct platform_driver user_drv = {
    .probe = si5351a_user_probe,
    .remove = si5351a_user_remove,
    .driver =
        {
            .name = "si5351a_usr_driver",
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(si5351a_of_table),
        },
    .id_table = si5351a_id_table,
};

static int __init user_output_init(void) {
  return platform_driver_register(&user_drv);
}

static void __exit user_output_exit(void) {
  platform_driver_unregister(&user_drv);
}

module_init(user_output_init);
module_exit(user_output_exit);
MODULE_LICENSE("GPL");