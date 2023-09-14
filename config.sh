#! /bin/bash


srcfile=`readlink -f $0`
srcdir=`dirname $srcfile`

buildcurrent=0
if [ $# -gt 0 ]
then
	if [ $1 = "--buildcurrent" ]
	then
		echo "buildcurrent"
		buildcurrent=1
	fi
fi

if [ -f $srcdir/build/build.ninja ]
then
#$srcdir/configure setup --reconfigure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
$srcdir/configure --reconfigure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
else
$srcdir/configure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
fi

if [ -f $srcdir/build/build.ninja ]
then
	
	if [ $buildcurrent -ne 0 ]
	then
		python $srcdir/config.py rpathrepl -i $srcdir/build/build.ninja -o $srcdir/build/build.ninja ''
	else
		python $srcdir/config.py rpathrepl -i $srcdir/build/build.ninja -o $srcdir/build/build.ninja
	fi
fi