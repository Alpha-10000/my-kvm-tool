#!/bin/sh

export ARCH=x86_64
export ROOTDIR=$(pwd)
export CFGDIR=$ROOTDIR/configs
export SRCDIR=$ROOTDIR/src
export BUILDIR=$ROOTDIR/build
export SYSROOT=$ROOTDIR/build/sysroot
export ROOTFS=$BUILDIR/rootfs

mkdir -p $SYSROOT $BUILDIR

# build linux
(
	cd $SRCDIR
	mkdir -p $BUILDIR/linux 
	cp $CFGDIR/linux-config $BUILDIR/linux/.config
	make -C $SRCDIR O=$BUILDIR/linux oldconfig
	make -C $SRCDIR O=$BUILDIR/linux -j4
	make -C $SRCDIR O=$BUILDIR/linux INSTALL_HDR_PATH=$SYSROOT/usr/ headers_install
)
