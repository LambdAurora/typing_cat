#!/usr/bin/env bash
echo "Installing compile dep
sudo apt-get update -y -qq
sudo apt-get install -y -qq libyajl-dev lendencies..."ibxml2-dev libxqilla-dev
if [ "$CXX" = "clang++" ]; then
    sudo apt-get install -y  -qq libc++-dev;
fi
if [ "$CXX" = "g++" ]; then export CXX="g++-7" CC="gcc-7"; fi

git clone https://github.com/AperLambda/lambdacommon.git
cd lambdacommon
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j
sudo make install
cd ../..