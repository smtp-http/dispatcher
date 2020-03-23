#!/bin/bash

rm -rf ./install 
mkdir ./install
cp -rf ./dispserver_d ./install
cp -rf ./log4cxx.cfg  ./install
cp -rf ./config.ini   ./install
cp -rf ./ReleaseNote.txt ./install

PRODUCT_NAME="DispatchServer"
MAJOR_VERSION="2"
SUB_VERSION="0"
THIRD_VERSION="1"
echo "product.name=${PRODUCT_NAME}" > ./install/version.ini
echo "product.major.version=${MAJOR_VERSION}"  >> ./install/version.ini
echo "product.sub.version=${SUB_VERSION}"  >> ./install/version.ini
echo "product.sub.version=${THIRD_VERSION}"  >> ./install/version.ini
SVN_BUILD_VERSION=`svn info https://10.1.10.4:8443/svn/MediaServer/Dispatcher/trunk|grep 'Last Changed Rev'| cut -d: -f2 |sed 's/^ //;s/ $//'`
echo "product.build.version=${SVN_BUILD_VERSION}"  >> ./install/version.ini
BUILD_TIME=`date '+%Y%m%d%H%M%S'`
echo "product.build.date=${BUILD_TIME}"  >> ./install/version.ini

cp -rf ./build/* ./install
rm -rf ./install/install.sh

tar zcvf install.tar.gz install 
 
rm -rf ./DispatchServer 
mkdir ./DispatchServer
cp -rf ./install.tar.gz ./DispatchServer
cp -rf ./build/install.sh ./DispatchServer
cp -rf ./build/pre_install_dispserver.sh ./DispatchServer
cp -rf ./build/util.functions.sh ./DispatchServer
cp -rf ./build/./supervisord.sh ./DispatchServer
cp -rf ./build/./mod_supervisord.py ./DispatchServer

packet_name="${PRODUCT_NAME}-V${MAJOR_VERSION}.${SUB_VERSION}.${THIRD_VERSION}.${SVN_BUILD_VERSION}.${BUILD_TIME}.linux.tar.gz"

echo ${packet_name}
tar zcvf ${packet_name} DispatchServer 
