#!/bin/bash

sudo insmod ./src/pci/rtsx_pci.ko
sudo insmod ./src/sdmmc/rtsx_pci_sdmmc.ko
sudo insmod ./src/memstick/rtsx_pci_ms.ko
