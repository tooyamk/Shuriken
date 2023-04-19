srkSetSubProject("")

set(installHostBinDir ${PROJECT_BINARY_DIR}/install/host/bin)

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_CPYTHON_NAME ${PROJECT_NAME})

if (SRK_HOST_OS_WINDOWS)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_CPYTHON_HOST_EXE ${installHostBinDir}/python.exe)
else ()
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_CPYTHON_HOST_EXE ${installHostBinDir}/python)
endif ()

set(major 3)
set(minor 11)
set(gitTag v${major}.${minor}.3)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/python/cpython.git
    GIT_TAG ${gitTag}
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ""
        ${CMAKE_COMMAND}
        -DSRK_HOST_OS_WINDOWS=${SRK_HOST_OS_WINDOWS}
        -DSRK_HOST_ARCH_X86=${SRK_HOST_ARCH_X86}
        -DSRK_HOST_ARCH_ARM=${SRK_HOST_ARCH_ARM}
        -DSRK_HOST_ARCH_WORD_BITS_32=${SRK_HOST_ARCH_WORD_BITS_32}
        -DSRK_HOST_ARCH_WORD_BITS_64=${SRK_HOST_ARCH_WORD_BITS_64}
        -DMAJOR=${major}
        -DMINOR=${minor}
        -DSRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/src
        -DINSTALL_DIR=${installHostBinDir}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/build.cmake
    INSTALL_COMMAND ""
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})