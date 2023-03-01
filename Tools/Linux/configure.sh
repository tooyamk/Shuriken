#!/bin/bash

for i in "$@"; do
    key=${i%%=*}
    val=${i#*=}
    if [ "${key}" = "BUILD_DIR" ]; then
        BUILD_DIR=${val}
    elif [ "${key}" = "BUILD_TYPE" ]; then
        BUILD_TYPE=${val}
    elif [ "${key}" = "SRK_ROOT" ]; then
        SRK_ROOT=${val}
    fi
done

while :
do
    if [ ! -n "${BUILD_DIR}" ]; then
        read -p BUILD_DIR= BUILD_DIR
        BUILD_DIR=`echo ${BUILD_DIR} | xargs`
        continue
    fi

    if [ ! -n "${BUILD_TYPE}" ]; then
        read -p BUILD_TYPE= BUILD_TYPE
        continue
    fi

    if [ "${BUILD_TYPE}" != "Debug" -a "${BUILD_TYPE}" != "Release" ]; then
        echo "BUILD_TYPE=${BUILD_TYPE}, not a valid value (Debug or Release)"
        read -p BUILD_TYPE= BUILD_TYPE
        continue
    fi

    if [ ! -n "${SRK_ROOT}" ]; then
        read -p SRK_ROOT= SRK_ROOT
        SRK_ROOT=`echo ${SRK_ROOT} | xargs`
        continue
    fi

    CMAKE_LISTS_FILE=${SRK_ROOT}/CMakeLists.txt
    if [ ! -f "${CMAKE_LISTS_FILE}" ]; then
        echo ${CMAKE_LISTS_FILE} is not exist
        read -p SRK_ROOT= SRK_ROOT
        continue
    fi

    break
done

buildDir=${BUILD_DIR}/${BUILD_TYPE}
rm -f "${buildDir}/CMakeCache.txt"
cmake -DSRK_ENABLE_TESTS=ON -DSRK_RPATH=".;./libs" -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B "${buildDir}" "${SRK_ROOT}"