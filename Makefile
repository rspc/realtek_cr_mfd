BUILD_DIR := /lib/modules/$(shell uname -r)/build/

#export CONFIG_CR_DEBUG = y

obj-m += src/

default:
	make -C $(BUILD_DIR) SUBDIRS=$(PWD) modules

clean:
	make -C $(BUILD_DIR) SUBDIRS=$(PWD) clean
