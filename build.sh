#!/bin/sh

export ARCH=arm

export CROSS_COMPILE=arm-fsl-linux-gnueabi-

make mx6ul_topeet_defconfig

make -j4
