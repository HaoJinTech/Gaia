########################################################
# put your Gaia git path and raspberrypi path here #####
export RASPBERRYPI_GIT_PATH=~/prj/git/Raspberry
export GAIA_GIT_PATH=~/prj/git/Gaia
#########################################################

export GAIA_BUILD_BASE=$(cd `dirname $0`; pwd)

export ARCH_HOME=/usr/src/aarch64-linux-gnu
export PATH=$PATH:$ARCH_HOME/bin
export KBUILD_OUTPUT=$GAIA_BUILD_BASE/build
export OUTPUT_IMAGE=$GAIA_BUILD_BASE/output_image

echo "===================PATH====================="
echo "RASPBERRYPI_GIT_PATH=$RASPBERRYPI_GIT_PATH"
echo "GAIA_GIT_PATH=$GAIA_GIT_PATH"
echo "GAIA_BUILD_BASE=$GAIA_BUILD_BASE"
echo "ARCH_HOME=$ARCH_HOME"
echo "KBUILD_OUTPUT=$KBUILD_OUTPUT"
echo "OUTPUT_IMAGE=$OUTPUT_IMAGE"
echo "============================================"

