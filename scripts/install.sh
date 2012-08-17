#!/bin/bash

DRV_TREE=/lib/modules/$(uname -r)/kernel/drivers

echo ""
echo "****************"
echo "*     Copy     *"
echo "****************"
echo ""
sudo cp -fv ./pci/rtsx_pci.ko $DRV_TREE/mfd/
sudo cp -fv ./sdmmc/rtsx_pci_sdmmc.ko $DRV_TREE/mmc/host/
echo ""
echo "****************"
echo "*    Depmod    *"
echo "****************"
echo ""
sudo depmod
echo "Done!"
