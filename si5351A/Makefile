# 驱动模块名称（不带.ko后缀）
MODULE_NAME := si5351A

SRC_DIRS := .
# 源文件（多个文件用空格分隔）
# CFILES := $(shell find $(SRC_DIRS) -name *.c)
# CPPFILES := $(shell find $(SRC_DIRS) -name *.cpp)
# SRCS += $(CFILES)
# SRCS += $(CPPFILES)
SRCS :=  si5351a.c

# 目标文件（自动从源文件生成）
OBJS := $(SRCS:.c=.o)

# 内核源码路径（可通过命令行覆盖）
#KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
KERNEL_DIR ?= /home/xiao/project/f1c100s/SDK/Linux
ccflags-y += "-I/home/xiao/tools/f1c100s_tools/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/lib/gcc/arm-linux-gnueabi/7.2.1/include/"
# 交叉编译工具链前缀（可通过命令行设置）
CROSS_COMPILE ?= /home/xiao/tools/f1c100s_tools/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-
ccflags-y += -I/home/xiao/project/f1c100s/SDK/Linux/drivers/staging/fbtft
# 目标架构（可通过命令行设置）
ARCH ?= arm

# # 自动获取工具链的 sysroot 和 include 路径
# SYSROOT := $(shell $(CROSS_COMPILE)gcc -print-sysroot 2>/dev/null)
# GCC_INCLUDE := $(shell $(CROSS_COMPILE)gcc -print-file-name=include 2>/dev/null)

# 编译选项
EXTRA_CFLAGS := -DDEBUG -Wall -Werror

# 添加自动检测的包含路径
# ifneq ($(SYSROOT),)
#     EXTRA_CFLAGS += --sysroot=$(SYSROOT)
# endif

# ifneq ($(GCC_INCLUDE),)
#     EXTRA_CFLAGS += -I$(GCC_INCLUDE)
# endif

# 内核模块构建目标
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := $(OBJS)

# 默认目标：构建模块
all:
	@echo "Building kernel module for $(shell uname -r)"
	@echo "KERNEL_DIR = $(KERNEL_DIR)"
	@echo "CROSS_COMPILE = $(CROSS_COMPILE)"
	@echo "ARCH = $(ARCH)"
	@echo "PWD = $(PWD)"
	@echo "EXTRA_CFLAGS = $(EXTRA_CFLAGS)"
	@echo "Source files: $(SRCS)"
	
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH)  \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

# 清理构建
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f *.order *.symvers

# 安装模块
install: all
	sudo insmod $(MODULE_NAME).ko

# 卸载模块
uninstall:
	sudo rmmod $(MODULE_NAME) || true

# 查看内核日志
log:
	dmesg | tail -20

# 显示帮助信息
help:
	@echo "Linux 驱动模块编译 Makefile"
	@echo "用法:"
	@echo "  make [选项] [目标]"
	@echo ""
	@echo "目标:"
	@echo "  all       默认目标，构建驱动模块 (默认)"
	@echo "  clean     清理构建产物"
	@echo "  install   安装并加载模块 (需要sudo)"
	@echo "  uninstall 卸载模块 (需要sudo)"
	@echo "  log       查看内核日志"
	@echo "  help      显示此帮助信息"
	@echo ""
	@echo "选项:"
	@echo "  KERNEL_DIR=path     指定内核源码路径"
	@echo "  CROSS_COMPILE=prefix 指定交叉编译工具链前缀"
	@echo "  ARCH=arch           指定目标架构 (如arm, arm64, x86等)"
	@echo ""
	@echo "交叉编译示例:"
	@echo "  make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KERNEL_DIR=/path/to/kernel"

# 显示内核配置状态
status:
	@echo "Kernel config status:"
	@if [ -L "$(KERNEL_DIR)/include/asm" ]; then \
		echo "  include/asm symlink exists: $(shell readlink $(KERNEL_DIR)/include/asm)"; \
	else \
		echo "  include/asm symlink MISSING!"; \
	fi
	@if [ -f "$(KERNEL_DIR)/.config" ]; then \
		echo "  Kernel config exists"; \
		grep CONFIG_ARM $(KERNEL_DIR)/.config; \
	else \
		echo "  Kernel config MISSING!"; \
	fi

.PHONY: all clean install uninstall log help