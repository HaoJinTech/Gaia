########################################################
# put your Gaia git path and raspberrypi path here #####
export RASPBERRYPI_GIT_PATH=~/prj/git/Raspberry
export GAIA_GIT_PATH=~/prj/git/Gaia
#########################################################

export GAIA_BUILD_PATH=$(cd `dirname $0`; pwd)

export ARCH_HOME=/usr/src/aarch64-linux-gnu
export PATH=$PATH:$ARCH_HOME/bin
export KBUILD_OUTPUT=$GAIA_BUILD_PATH/kernal/build

echo "===================PATH====================="
echo "RASPBERRYPI_GIT_PATH=$RASPBERRYPI_GIT_PATH"
echo "GAIA_GIT_PATH=$GAIA_GIT_PATH"
echo "GAIA_BUILD_PATH=$GAIA_BUILD_PATH"
echo "ARCH_HOME=$ARCH_HOME"
echo "KBUILD_OUTPUT=$KBUILD_OUTPUT"
echo "============================================"

