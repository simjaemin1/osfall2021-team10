#!/bin/bash


echo "******building kernel...******"
./build-rpi3-arm64.sh
echo "******making kernel image files...******"
sudo ./scripts/mkbootimg_rpi3.sh
echo "******moving image files to tizen-image...******"
cp ../osfall2021/src/qemu/qemu.sh
cp ../osfall2021/src/qemu/tizen-bcmrpi3-defconfig arch/arm64/configs/
rm -f ../tizen-image/boot.img
rm -f ../tizen-image/modules.img
mv boot.img ../tizen-image
mv modules.img ../tizen-image
echo "******booting tizen...******"
sudo ./qemu.sh


