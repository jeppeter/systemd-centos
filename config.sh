#! /bin/bash

source extargsparse4sh

srcfile=`readlink -f $0`
srcdir=`dirname $srcfile`

read -r -d '' OPTIONS <<EOFM
{
	"verbose|v" : "+",
	"buildcurrent|B##to make build systemd as in the current running environment default false##" : false
}
EOFM

parse_command_line "$OPTIONS" "$@"

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
		echo "build current"
		python $srcdir/config.py rpathrepl -i $srcdir/build/build.ninja -o $srcdir/build/build.ninja ''
	else
		python $srcdir/config.py rpathrepl -i $srcdir/build/build.ninja -o $srcdir/build/build.ninja
	fi
fi