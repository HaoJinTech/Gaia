source 000_pathdef.sh

cd $GAIA_BUILD_PATH/kernal/linux_src
make -j6 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
