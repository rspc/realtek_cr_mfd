BUILD_DIR := /lib/modules/$(shell uname -r)/build/

#subdir-ccflags-y := -DDEBUG

obj-m += pci/
obj-m += sdmmc/

default:
	make -C $(BUILD_DIR) SUBDIRS=$(PWD) modules

clean:
	make -C $(BUILD_DIR) SUBDIRS=$(PWD) clean
