source 000_pathdef.sh

cd $GAIA_BUILD_BASE/linux_src

make distclean
make ARCH=arm64 mrproper
