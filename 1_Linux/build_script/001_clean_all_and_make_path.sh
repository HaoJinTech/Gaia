source 000_pathdef.sh

CRTDIR=$(pwd)

if [ -d "${CRTDIR}/kernal" ]; then
  rm -rf ${CRTDIR}/kernal
fi

if [ -d "${CRTDIR}/rootfs" ]; then
  rm -rf ${CRTDIR}/rootfs
fi

mkdir ${CRTDIR}/kernal
mkdir ${CRTDIR}/rootfs

KERNAL_BUILD_PATH=${CRTDIR}/kernal

if [ ! -d "$KBUILD_OUTPUT" ]; then
  echo "mkdir:$KBUILD_OUTPUT ..."
  mkdir $KBUILD_OUTPUT
fi

# make a soft link for linux kernal src code got from git
ln -s $RASPBERRYPI_GIT_PATH/linux $KERNAL_BUILD_PATH/linux_src


