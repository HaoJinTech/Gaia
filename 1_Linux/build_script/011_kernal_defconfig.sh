source 000_pathdef.sh

echo "enthering path: $GAIA_BUILD_BASE/linux_src ..."
cd $GAIA_BUILD_BASE/linux_src
echo "start config ..."
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig
echo "done."
