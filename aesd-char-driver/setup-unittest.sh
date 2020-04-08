#!/bin/bash
cd `dirname $0`
sudo apt-get install cmake ruby
git submodule init 
git submodule update
mkdir -p build 
cd build
cmake ..
