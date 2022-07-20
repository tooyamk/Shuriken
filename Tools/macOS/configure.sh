#!/bin/sh
buildDir=../../build

rm -f ${buildDir}/CMakeCache.txt
c=`which clang`
cxx=`which clang++`
cmake -DSRK_TESTS=ON -DCMAKE_INSTALL_PREFIX=install -DCMAKE_C_COMPILER=$c -DCMAKE_CXX_COMPILER=$cxx -G Xcode -B $buildDir ../..