#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>

int si5351a_user_probe(struct platform_device *pdev) {
  struct clk *clk;
  clk = clk_get(NULL, "my_gateable_clk");
  return 0;
}

int si5351a_user_remove(struct platform_device *pdev) {

  return 0;
}

static const struct platform_device_id si5351a_id_table[] = {
    [0] = {"si5351a-usr", 0}, // 用于匹配
    [1] = {},                 // 表示结束
};

static const struct of_device_id si5351a_of_table[] = {
    {.compatible = "Silicon,si5351a-usr"}, {}};

static struct platform_driver si5351a_user_drv = {
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

static int __init si5351a_user_output_init(void) {
  return platform_driver_register(&si5351a_user_drv);
}

static void __exit si5351a_user_output_exit(void) {
  platform_driver_unregister(&si5351a_user_drv);
}

module_init(si5351a_user_output_init);
module_exit(si5351a_user_output_exit);
MODULE_LICENSE("GPL");