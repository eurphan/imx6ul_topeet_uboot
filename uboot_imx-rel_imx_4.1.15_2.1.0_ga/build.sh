#!/bin/sh

export ARCH=arm

export CROSS_COMPILE=arm-linux-gnueabihf-

make mx6ul_topeet_defconfig

make -j4
