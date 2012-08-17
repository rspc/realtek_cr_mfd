#!/bin/bash

LINK_DIR=/lib/modules/$(uname -r)/build/include/linux/mfd
TARGET_DIR=$(pwd)/include

if [ -e $LINK_DIR/rtsx_pci.h ]; then
	sudo rm -v $LINK_DIR/rtsx_pci.h
fi
if [ -e $LINK_DIR/rtsx_common.h ]; then
	sudo rm -v $LINK_DIR/rtsx_common.h
fi

sudo ln -v -s $TARGET_DIR/rtsx_pci.h $LINK_DIR/rtsx_pci.h
sudo ln -v -s $TARGET_DIR/rtsx_common.h $LINK_DIR/rtsx_common.h

