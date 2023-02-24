#!/bin/sh

for i in "$@"; do
    key=${i%%=*}
    val=${i#*=}
    if [[ "${key}" == "BUILD_DIR" ]]; then
        BUILD_DIR=${val}
    elif [[ "${key}" == "SRK_ROOT" ]]; then
        SRK_ROOT=${val}
    fi
done

while :
do
    if [[ "${BUILD_DIR}" == "" ]]; then
        read -p BUILD_DIR= BUILD_DIR
        continue
    fi

    if [[ "${SRK_ROOT}" == "" ]]; then
        read -p SRK_ROOT= SRK_ROOT
        continue
    fi

    CMAKE_LISTS_FILE=${SRK_ROOT}/CMakeLists.txt
    if [[ ! -f "${CMAKE_LISTS_FILE}" ]]; then
        echo ${CMAKE_LISTS_FILE} is not exist
        read -p SRK_ROOT= SRK_ROOT
        continue
    fi

    break
done

rm -f "${BUILD_DIR}/CMakeCache.txt"
c=`which clang`
cxx=`which clang++`
cmake -DSRK_ENABLE_TESTS=ON -DSRK_RPATH=".;./libs;./../../../libs" -DCMAKE_INSTALL_PREFIX=install -DCMAKE_C_COMPILER="${c}" -DCMAKE_CXX_COMPILER="${cxx}" -G Xcode -B "${BUILD_DIR}" "${SRK_ROOT}"