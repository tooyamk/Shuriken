#!/bin/bash

buildDir=../../build

#buildDir, buildType
configure() {
    local buildType=$2
    local dir=${1}/${buildType}
    rm -f ${dir}/CMakeCache.txt
    cmake -DSRK_TESTS=ON -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$buildType -B $dir ../..
}

configure $buildDir Debug
configure $buildDir Release