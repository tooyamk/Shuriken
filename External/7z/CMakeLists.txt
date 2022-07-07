srkSetProject()

set(installDir ${PROJECT_BINARY_DIR}/install)

set_property(GLOBAL PROPERTY SRKPROJ_7Z_NAME ${PROJECT_NAME})

if (SRK_HOST_OS_WINDOWS)
    set_property(GLOBAL PROPERTY SRKPROJ_HOST_7Z_EXE ${installDir}/7zz.exe)
elseif (SRK_HOST_OS_MACOS)
    set_property(GLOBAL PROPERTY SRKPROJ_HOST_7Z_EXE ${installDir}/7zz)
endif ()

set(vcvarsall "nil")
if (SRK_CXX_COMPILER_MSVC)
    srkFindParentDir(${CMAKE_CXX_COMPILER} vcvarsall "VC")
    if (NOT (${vcvarsall} STREQUAL "nil"))
        set(vcvarsall ${vcvarsall}/Auxiliary/Build/vcvarsall.bat)
    endif ()
endif ()

set(projDir ${PROJECT_BINARY_DIR}/src/CPP/7zip/Bundles/Alone2)

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://www.7-zip.org/a/7z2200-src.7z
    URL_HASH SHA256=aa875031817c65b8e1069c5bfa12fa0176ba01431f78489656b411cc2e7c572c
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_BUILD_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    PATCH_COMMAND
        ${CMAKE_COMMAND}
        -DSRK_HOST_OS_MACOS=${SRK_HOST_OS_MACOS}
        -DPROJ_DIR=${projDir}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch.cmake
    BUILD_COMMAND
        ${CMAKE_COMMAND}
        -DSRK_HOST_OS_WINDOWS=${SRK_HOST_OS_WINDOWS}
        -DSRK_HOST_OS_MACOS=${SRK_HOST_OS_MACOS}
        -DCMAKE_CUR_SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DPROJ_DIR=${projDir}
        -DBUILD_DIR=${SRK_BUILD_DIR}
        -DVCVARSALL=${vcvarsall}
        -DSRK_CXX_COMPILER_MSVC=${SRK_CXX_COMPILER_MSVC}
        -DSRK_HOST_ARCH_X86=${SRK_HOST_ARCH_X86}
        -DSRK_HOST_ARCH_ARM=${SRK_HOST_ARCH_ARM}
        -DSRK_HOST_ARCH_WORD_BITS_32=${SRK_HOST_ARCH_WORD_BITS_32}
        -DSRK_HOST_ARCH_WORD_BITS_64=${SRK_HOST_ARCH_WORD_BITS_64}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/build.cmake
    INSTALL_COMMAND
        ${CMAKE_COMMAND}
        -DBUILD_DIR=${SRK_BUILD_DIR}
        -DINSTALL_DIR=${installDir}
        -DSRK_HOST_OS_WINDOWS=${SRK_HOST_OS_WINDOWS}
        -DSRK_HOST_OS_MACOS=${SRK_HOST_OS_MACOS}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/install.cmake
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})