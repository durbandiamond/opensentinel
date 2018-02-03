#!/bin/bash

set -e 

date
ps axjf

echo $1

#################################################################
# Build from source                                           #
#################################################################
NPROC=$(nproc)
echo "nproc: $NPROC"
OPENSENT_ROOT=$(pwd)

#################################################################
# Install all necessary packages for building                 #
#################################################################
#sudo apt-get -y install build-essential
#sudo apt-get update

# Use core count -1 threads.
nproc=$(nproc)
if [ $nproc -eq 1 ]
then
	((job=nproc))
elif [ $nproc -gt 1 ]
then
	((job=nproc-1))
fi

echo "Using $job thread(s)"

mkdir deps
cd deps

##
# ASIO
##

wget --no-check-certificate "https://pilotfiber.dl.sourceforge.net/project/asio/asio/1.10.8%20%28Stable%29/asio-1.10.8.tar.gz"
tar -xzf asio-1.10.8.tar.gz
rm -rf asio-1.10.8.tar.gz
mv asio-1.10.8 asio

##
# Boost Build System (bjam).
##

wget "https://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz"
tar -xzf boost_1_66_0.tar.gz
rm -rf boost_1_66_0.tar.gz
mv boost_1_66_0 boost
cd boost
./bootstrap.sh
cd $OPENSENT_ROOT

cd test
../deps/boost/bjam -j$job toolset=gcc cxxflags="-std=gnu++0x -fpermissive" release
cd $OPENSENT_ROOT

cp test/bin/gcc-*/release/link-static/stack $OPENSENT_ROOT/bin/opensentinel

exit 0

