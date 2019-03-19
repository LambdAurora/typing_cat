#!/usr/bin/env bash
echo "Installing compile dependencies..."
#brew update
if [ "$CXX" = "g++" ]; then
    echo "Installing GCC..."
    #brew install gcc
fi

if [ "$CXX" = "g++" ]; then export CXX="g++" CC="gcc"; fi

git clone https://github.com/AperLambda/lambdacommon.git
cd lambdacommon
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j
sudo make install
cd ../..