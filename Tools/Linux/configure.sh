#!/bin/sh
rm -f ./CMakeCache.txt
cmake -DSRK_TESTS=ON -DCMAKE_INSTALL_PREFIX="install" -DCMAKE_BUILD_TYPE=Debug ..