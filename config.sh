#! /bin/bash

srcfile=`readlink -f $0`
srcdir=`dirname $srcfile`

if [ -f $srcdir/build/build.ninja ]
then
$srcdir/configure setup --reconfigure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
else
$srcdir/configure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
fi

if [ -f $srcdir/build/build.ninja ]
then
	python $srcdir/config.py rpathrepl -i $srcdir/build/build.ninja -o $srcdir/build/build.ninja
fi