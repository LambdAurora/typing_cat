#!/bin/sh

# Some color variables.
RED='\033[0;91m'
NC='\033[0m'

mkdir -p build/
cd build/
echo "Building project..."
cmake ..
if [ $? -ne 0 ]; then
echo "${RED}Error: CMake doesn't exit with success! Cleaning...${NC}"
cd ..
else
make -j
if [ $? -ne 0 ]; then
echo "${RED}Error: Make doesn't exit with success! Cleaning...${NC}"
cd ..
fi
fi

cd ..