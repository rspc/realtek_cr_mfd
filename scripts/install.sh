#!/bin/bash

DRV_TREE=/lib/modules/$(uname -r)/kernel/drivers

echo ""
echo "****************"
echo "*     Copy     *"
echo "****************"
echo ""
sudo cp -fv ./src/pci/rtsx_pci.ko $DRV_TREE/mfd/
sudo cp -fv ./src/sdmmc/rtsx_pci_sdmmc.ko $DRV_TREE/mmc/host/
sudo cp -fv ./src/memstick/rtsx_pci_ms.ko $DRV_TREE/memstick/host/
echo ""
echo "****************"
echo "*    Depmod    *"
echo "****************"
echo ""
sudo depmod
echo "Done!"
