source 000_pathdef.sh

CRTDIR=$(pwd)

if [ -d "$KBUILD_OUTPUT" ]; then
  echo "rm $KBUILD_OUTPUT ..."
  rm -rf $KBUILD_OUTPUT
fi

mkdir $KBUILD_OUTPUT

if [ -d "$OUTPUT_IMAGE" ]; then
  echo "rm $OUTPUT_IMAGE ..."
  rm -rf $OUTPUT_IMAGE
fi

mkdir $OUTPUT_IMAGE

# make a soft link for linux kernal src code got from git
if [ ! -d "$CRTDIR/linux_src" ]; then
  echo "link $CRTDIR/linux_src ..."
  ln -s $RASPBERRYPI_GIT_PATH/linux $CRTDIR/linux_src
fi

# make a soft link for busybox src code got from git
if [ ! -d "$CRTDIR/busybox_src" ]; then
  echo "link $CRTDIR/busybox_src ..."
  ln -s $GAIA_GIT_PATH/1_Linux/rootfs/busybox_src $CRTDIR/busybox_src
fi


