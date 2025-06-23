#include <linux/clk.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#define SI5351A_CLASS_NAME "si5351x_Out"
#define SI5351A_DEV_NAME  "si5351x"

struct output_clock_device {
  struct device *dev;
  struct clk **clks;
  int num_clks;
};
static struct output_clock_device *clk_dev;

int si5351a_user_probe(struct platform_device *pdev) {
  struct clk *clk;
  const char *const *clk_names = NULL;
  int i, count;

  clk_dev = devm_kzalloc(&pdev->dev, sizeof(*clk_dev), GFP_KERNEL);
  if (!clk_dev)
    return -ENOMEM;

  clk_dev->dev = &pdev->dev;

  /* first, get clocks from device tree */
  count =
      of_count_phandle_with_args(pdev->dev.of_node, "clocks", "#clock-cells");
  if (count <= 0) {
    dev_err(&pdev->dev, "No clocks defined in device tree\n");
    goto err_free;
  }
  clk_dev->num_clks = count;

  clk_names = of_get_property(pdev->dev.of_node, "clock-names", NULL);
  if (!clk_names) {
    pr_err("No clock-names property found\n");
    goto err_free;
  }

  for (i = 0; i < count; i++) {
    clk = devm_clk_get(&pdev->dev, clk_names[i]);
    if (IS_ERR(clk)) {
      dev_err(&pdev->dev, "Failed to get clock %d\n", i);
      return PTR_ERR(clk);
    }
    dev_info(&pdev->dev, "Clock %d: %s\n", i, clk_names[i]);
    clk_dev->clks[i] = clk;
    clk_unprepare(clk);
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
  return -ENOMEM;
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