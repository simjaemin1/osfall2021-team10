#!/bin/bash

sudo ./build-rpi3-arm64.sh;
sudo ./scripts/mkbootimg_rpi3.sh;
sudo mv boot.img modules.img ../tizen-image;
sudo mount ../tizen-image/rootfs.img ../mnt_dir;
cd test; arm-linux-gnueabi-gcc -I../include test.c -o test; cd ..;
cd test; arm-linux-gnueabi-gcc -I../include test_fork.c -o test_fork; cd ..;
sudo cp test/test ../mnt_dir/root;
sudo cp test/test_fork ../mnt_dir/root;
sudo sh test.sh
