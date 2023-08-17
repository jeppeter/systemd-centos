#! /bin/bash

if [ -f build/build.ninja ]
then
./configure setup --reconfigure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
else
./configure --buildtype release -Dkmod=true -Dacl=true -Dlibcryptsetup=true -Dgcrypt=true -Dlibidn2=true -Dxz=true -Dzlib=true -Dseccomp=true -Dselinux=true 
fi

if [ -f build/build.ninja ]
then
	python ./config.py rpathrepl -i build/build.ninja -o build/build.ninja
fi