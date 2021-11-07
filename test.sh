#!/bin/zsh
cd test; arm-linux-gnueabi-gcc -I../include func.c -o func; cd ..;
cd test; arm-linux-gnueabi-gcc -I../include prime_factor.c -o pf; cd ..;
cd test; arm-linux-gnueabi-gcc -I../include prime_factor_single.c -o pfs; cd ..;
cd test; arm-linux-gnueabi-gcc -I../include pf_lb.c -o pflb; cd ..;
cd test; arm-linux-gnueabi-gcc -I../include bal.c -o bal; cd ..;


echo "COMPILE SUCCESS"
sudo cp test/func ../mnt_dir/root;
sudo cp test/pf ../mnt_dir/root;
sudo cp test/pfs ../mnt_dir/root;
sudo cp test/pflb ../mnt_dir/root;
sudo cp test/bal ../mnt_dir/root;
echo "CP SUCCESS"
sudo ./qemu.sh
