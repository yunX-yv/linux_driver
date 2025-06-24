#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#define SI5351A_CLASS_NAME "si5351x_Out"
#define SI5351A_DEV_NAME "si5351x"

struct output_clock_device {
  struct device *dev;
  struct clk **clks;
  struct device **devs;
  struct cdev *cdev;
  int num_clks;
  struct class *cls;
  int major;
  int minor;
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
    clk_set_rate(clk_dev->clks[0], 1000000); // 设置第一个时钟频率为1MHz
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
  struct clk *clk = NULL;
  int i = 0, count = 0;
  struct cdev *cdev = NULL;
  int ret;
  dev_t dev_c;
  struct class *cls = NULL;
  int major = 0; // 获取申请到的主设备号
  int minor = 0; // 获取申请到的次设备号
  struct device *dev;

  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  clk_dev = devm_kzalloc(&pdev->dev, sizeof(*clk_dev), GFP_KERNEL);
  if (!clk_dev) {
    dev_err(&pdev->dev, "devm_kzalloc err\n");
    return -ENOMEM;
  }

  clk_dev->dev = &pdev->dev;
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  /* first, get clocks from device tree */
  count = of_property_count_u32_elems(pdev->dev.of_node, "clocks");
  if (count <= 0) {
    dev_err(&pdev->dev, "No clocks defined in device tree\n");
    ret = -ENOMEM;
    goto err_free;
  }
  clk_dev->num_clks = count;

  // printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  // clk_names = of_get_property(pdev->dev.of_node, "clock-names", NULL);
  // if (!clk_names) {
  //   dev_err(&pdev->dev, "No clock-names property found\n");
  //   ret = -ENOMEM;
  //   goto err_free;
  // }
  dev_info(&pdev->dev, "[%s:%d]clk_dev->num_clks %d\n", __func__, __LINE__,
           clk_dev->num_clks);

  // for (i = 0; i < clk_dev->num_clks; i++) {
  //   printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  //   clk = of_clk_get(pdev->dev.of_node, i);
  //   if (IS_ERR(clk)) {
  //     dev_err(&pdev->dev, "Failed to get clock %d\n", i);
  //     ret = -ENOMEM;
  //     goto err_free;
  //   }
  //   clk_dev->clks[i] = clk;
  //   dev_info(&pdev->dev, "clk_prepare\n");
  //   // clk_unprepare(clk);
  //   clk_prepare(clk);
  // }

  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  clk = devm_clk_get(&pdev->dev, "clkout0");
  if (IS_ERR(clk)) {
    dev_err(&pdev->dev, "Failed to get clock %d\n", i);
    ret = -ENOMEM;
    goto err_free;
  }
  dev_info(&pdev->dev, "clk_prepare\n");
  // clk_unprepare(clk);
  clk_prepare(clk);
  clk_dev->num_clks = 1; // 只使用一个时钟

  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  cdev = cdev_alloc(); // 字符设备结构体申请内存
  if (cdev == NULL) {
    dev_err(&pdev->dev, "[%s,%s,%d]cdev_alloc err\n", __FILE__, __func__,
            __LINE__);
    ret = -ENOMEM;
    goto err_free;
  }
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  cdev_init(cdev, &si5351a_fops); // 初始化字符设备结构体

  ret = alloc_chrdev_region(&dev_c, 0, clk_dev->num_clks, SI5351A_DEV_NAME);
  if (ret) {
    dev_err(&pdev->dev, "[%s,%s,%d]alloc_chrdev_region err\n", __FILE__,
            __func__, __LINE__);
    ret = -ENOMEM;
    goto err_free;
  }
  major = MAJOR(dev_c); // 获取申请到的主设备号
  minor = MINOR(dev_c); // 获取申请到的次设备号
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  clk_dev->major = major; // 保存主设备号
  clk_dev->minor = minor; // 保存次设备号

  ret = cdev_add(cdev, MKDEV(major, minor),
                 clk_dev->num_clks); // 按设备号注册设备
  if (ret) {
    dev_err(&pdev->dev, "[%s,%s,%d]cedv_add err\n", __FILE__, __func__,
            __LINE__);
    ret = -ENOMEM;
    goto err_free;
  }
  clk_dev->cdev = cdev;
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  // 2.注册设备节点
  cls = class_create(THIS_MODULE, SI5351A_CLASS_NAME); // 提交设备节点目录
  if (IS_ERR(cls)) {
    dev_err(&pdev->dev, "[%s,%s,%d]class_create err\n", __FILE__, __func__,
            __LINE__);
    ret = PTR_ERR(cls);
    goto err_free;
  }
  clk_dev->cls = cls;
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  for (i = 0; i < clk_dev->num_clks; i++) {
    // 提交设备节点文件名
    printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
    if (!cls)
      dev_err(&pdev->dev, "[%s,%d]cls err\n", __func__, __LINE__);

    dev = device_create(cls, NULL, MKDEV(major, i), NULL, "si5351a_out%d", i);
    if (IS_ERR(dev)) {
      dev_err(&pdev->dev, "[%s,%s,%d]device_create err\n", __FILE__, __func__,
              __LINE__);
      ret = PTR_ERR(dev);
      goto err_free;
    }
    printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
    clk_dev->devs[i] = dev;
  }
  printk("[%s:%s:%d]\n", __FILE__, __func__, __LINE__);
  return 0;

err_free:
  for (i = 0; i < clk_dev->num_clks; i++) {
    if (cls)
      device_destroy(cls, MKDEV(major, i)); // 注销设备节点文件名
  }
  if (cls)
    class_destroy(cls); // 注销设备节点目录

  if (cdev)
    cdev_del(cdev); // 注销字符设备

  unregister_chrdev_region(MKDEV(major, minor),
                           clk_dev->num_clks); // 注销设备号

  if (cdev)
    kfree(cdev); // 清除字符设备结构体的内存

  for (i = 0; i < clk_dev->num_clks; i++) {
    if (clk_dev->clks[i]) {
      clk_unprepare(clk_dev->clks[i]);
      devm_clk_put(&pdev->dev, clk_dev->clks[i]);
    }
  }
  devm_kfree(&pdev->dev, clk_dev);
  return ret;
}

int si5351a_user_remove(struct platform_device *pdev) {
  int i;
  for (i = 0; i < clk_dev->num_clks; i++) {
    if (clk_dev->cls)
      device_destroy(clk_dev->cls,
                     MKDEV(clk_dev->major, i)); // 注销设备节点文件名
  }
  if (clk_dev->cls)
    class_destroy(clk_dev->cls); // 注销设备节点目录

  if (clk_dev->cdev)
    cdev_del(clk_dev->cdev); // 注销字符设备

  unregister_chrdev_region(MKDEV(clk_dev->major, clk_dev->minor),
                           clk_dev->num_clks); // 注销设备号

  if (clk_dev->cdev)
    kfree(clk_dev->cdev); // 清除字符设备结构体的内存

  for (i = 0; i < clk_dev->num_clks; i++) {
    if (clk_dev->clks[i])
      devm_clk_put(&pdev->dev, clk_dev->clks[i]);
  }
  devm_kfree(&pdev->dev, clk_dev->clks);
  devm_kfree(&pdev->dev, clk_dev->clks);
  return 0;
}

static const struct platform_device_id si5351a_id_table[] = {
    [0] = {"si5351a-usr", 0}, // 用于匹配
    [1] = {},                 // 表示结束
};

static const struct of_device_id si5351a_of_table[] = {
    {.compatible = "si5351a-usr"}, {}};

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