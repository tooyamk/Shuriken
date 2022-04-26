#!/bin/sh
rm -f ./CMakeCache.txt
cmake -DAE_TESTS=ON -DCMAKE_INSTALL_PREFIX="install" -DCMAKE_BUILD_TYPE=Debug ..