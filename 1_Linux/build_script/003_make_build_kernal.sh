export ARCH_HOME=/usr/src/aarch64-linux-gnu
export PATH=$PATH:$ARCH_HOME/bin

make -j6 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
