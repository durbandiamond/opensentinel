#!/bin/bash

set -e 

date
ps axj

echo $1

OPENSENT_ROOT=$(pwd)

mkdir deps
cd deps

##
# ASIO
##

curl -L -O --insecure "https://pilotfiber.dl.sourceforge.net/project/asio/asio/1.10.8%20%28Stable%29/asio-1.10.8.tar.gz"
tar -xzf asio-1.10.8.tar.gz
rm -rf asio-1.10.8.tar.gz
mv asio-1.10.8 asio

##
# Boost Build System (bjam).
##

curl -L -O "https://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz"
tar -xzf boost_1_66_0.tar.gz
rm -rf boost_1_66_0.tar.gz
mv boost_1_66_0 boost
cd boost
./bootstrap.sh
./bjam link=static toolset=clang cxxflags="-std=c++11 -stdlib=libc++" --with-system release
cd $OPENSENT_ROOT

cd test

../deps/boost/bjam toolset=clang cxxflags="-std=c++11 -stdlib=libc++" release

cd $OPENSENT_ROOT

cp test/bin/clang-darwin-*/release/link-static/stack $OPENSENT_ROOT/bin/opensentinel

exit 0

