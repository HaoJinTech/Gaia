source 000_pathdef.sh

cd $GAIA_BUILD_BASE/linux_src
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- menuconfig
