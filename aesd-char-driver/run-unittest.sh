#!/bin/bash
set -e
cd `dirname $0`
cd build
make
./unity-test
