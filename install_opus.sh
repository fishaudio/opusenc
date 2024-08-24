#!/bin/bash
set -e

sudo apt install libopus-dev libopusfile-dev

# OGG and OpenSSL are required to install.
mkdir -p libs
cd libs

if [ ! -d "libopusenc-0.2.1" ]; then
    wget http://downloads.xiph.org/releases/opus/libopusenc-0.2.1.tar.gz
    tar -xf libopusenc-0.2.1.tar.gz
fi
cd libopusenc-0.2.1

./configure
make -j
sudo make install
sudo ldconfig

cd ..
