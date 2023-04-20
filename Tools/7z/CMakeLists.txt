srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

set(exe 7zz)
if (SRK_HOST_OS_WINDOWS)
    set(exe ${exe}.exe)
endif ()

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_7Z_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_7Z_EXE ${PROJECT_BINARY_DIR}/install/bin/${exe})

set(vcvarsall "nil")
if (SRK_CXX_COMPILER_MSVC)
    srkFindParentDir(${CMAKE_CXX_COMPILER} vcvarsall "VC")
    if (NOT (${vcvarsall} STREQUAL "nil"))
        set(vcvarsall ${vcvarsall}/Auxiliary/Build/vcvarsall.bat)
    endif ()
endif ()

set(projDir ${PROJECT_BINARY_DIR}/src/CPP/7zip/Bundles/Alone2)

set(fileName 7z2201-src.7z)
srkCheckExternalProjectVersion_DownloadFile(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/download/${fileName})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://www.7-zip.org/a/${fileName}
    URL_HASH SHA256=24cb2b9c6c0fb87d9c77a8d2afb572f5c75b45c950eff81a37ebac7fa73d1b4a
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
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
        -DINSTALL_DIR=${PROJECT_BINARY_DIR}/install/bin
        -DVCVARSALL=${vcvarsall}
        -DSRK_CXX_COMPILER_MSVC=${SRK_CXX_COMPILER_MSVC}
        -DSRK_HOST_ARCH_X86=${SRK_HOST_ARCH_X86}
        -DSRK_HOST_ARCH_ARM=${SRK_HOST_ARCH_ARM}
        -DSRK_HOST_ARCH_WORD_BITS_32=${SRK_HOST_ARCH_WORD_BITS_32}
        -DSRK_HOST_ARCH_WORD_BITS_64=${SRK_HOST_ARCH_WORD_BITS_64}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/build.cmake
    INSTALL_COMMAND ""
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOLS_DIR_NAME})