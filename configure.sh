#!/bin/bash

if [ -z "$2" ]; then
	ARCH="rv32im_zicond_zicsr_zifencei"
else
	ARCH=$2
fi

xmake f -a $ARCH -m $1 -c -p bare-metal
