source 000_pathdef.sh

cd $GAIA_BUILD_BASE/busybox_src

if [ ! -d "$OUTPUT_IMAGE/busybox_out" ]; then
  echo "rm $OUTPUT_IMAGE/busybox_out ..."
  mkdir $OUTPUT_IMAGE/busybox_out
fi
make install CONFIG_PREFIX=$OUTPUT_IMAGE/busybox_out
