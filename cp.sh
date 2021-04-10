#!/bin/sh

TARGET_PATH=~/srv/tftp

echo "copying u-boot.imx..."

if [ ! -d ${TARGET_PATH} ]; then
	echo "target path not exist, created!, "
	mkdir -p ${TARGET_PATH}
fi

cp u-boot.imx ${TARGET_PATH}
