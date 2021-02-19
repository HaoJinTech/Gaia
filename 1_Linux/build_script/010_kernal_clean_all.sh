source 000_pathdef.sh

cd kernal/linux_src

make distclean
make ARCH=arm64 mrproper
