srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

set(installDir ${PROJECT_BINARY_DIR}/install)
set(exeName python)
if (SRK_HOST_OS_WINDOWS)
    set(exeName ${exeName}.exe)
else ()
    set(exeName ${exeName}3)
endif ()
set(exe ${installDir}/bin/${exeName})

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_CPYTHON_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_CPYTHON_EXE ${exe})

get_property(SRKPROJ_TOOL_OPENSSL_NAME GLOBAL PROPERTY "SRKPROJ_TOOL_OPENSSL_NAME")
get_property(SRKPROJ_TOOL_OPENSSL_INCLUDE GLOBAL PROPERTY "SRKPROJ_TOOL_OPENSSL_INCLUDE")
get_property(SRKPROJ_TOOL_OPENSSL_LIB_DIR GLOBAL PROPERTY "SRKPROJ_TOOL_OPENSSL_LIB_DIR")

get_property(SRKPROJ_TOOL_ZLIB_NAME GLOBAL PROPERTY "SRKPROJ_TOOL_ZLIB_NAME")
get_property(SRKPROJ_TOOL_ZLIB_INCLUDE GLOBAL PROPERTY "SRKPROJ_TOOL_ZLIB_INCLUDE")
get_property(SRKPROJ_TOOL_ZLIB_LIB_DIR GLOBAL PROPERTY "SRKPROJ_TOOL_ZLIB_LIB_DIR")

set(major 3)
set(minor 11)
set(gitTag v${major}.${minor}.3)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

set(pipUpgrade ${exe} -m pip install -U pip)

include(ExternalProject)

set(externalProjectCmd
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
    UPDATE_COMMAND COMMAND
)

if (SRK_HOST_OS_WINDOWS)
    list(APPEND externalProjectCmd
        CONFIGURE_COMMAND COMMAND
        BUILD_COMMAND
            ${CMAKE_COMMAND}
            -DSRK_HOST_ARCH_X86=${SRK_HOST_ARCH_X86}
            -DSRK_HOST_ARCH_ARM=${SRK_HOST_ARCH_ARM}
            -DSRK_HOST_ARCH_WORD_BITS_32=${SRK_HOST_ARCH_WORD_BITS_32}
            -DSRK_HOST_ARCH_WORD_BITS_64=${SRK_HOST_ARCH_WORD_BITS_64}
            -DMAJOR=${major}
            -DMINOR=${minor}
            -DSRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/src
            -DBUILD_DIR=${SRK_PROJECT_BUILD_ROOT_DIR}
            -DINSTALL_DIR=${installDir}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/build.cmake
        INSTALL_COMMAND
            ${CMAKE_COMMAND}
            -DMAJOR=${major}
            -DMINOR=${minor}
            -DPYTHON_EXE=${exe}
            -DINSTALL_DIR=${installDir}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/install.cmake &&
            ${pipUpgrade}
    )

    ExternalProject_Add(${externalProjectCmd})
elseif (SRK_HOST_OS_LINUX OR SRK_HOST_OS_MACOS)
    list(APPEND externalProjectCmd
        PATCH_COMMAND 
            ${CMAKE_COMMAND}
            -DOPENSSL_INCLUDES=${SRKPROJ_TOOL_OPENSSL_INCLUDE}
            -DOPENSSL_LDFLAGS=${SRKPROJ_TOOL_OPENSSL_LIB_DIR}
            -DZLIB_INCLUDES=${SRKPROJ_TOOL_ZLIB_INCLUDE}
            -DZLIB_LDFLAGS=${SRKPROJ_TOOL_ZLIB_LIB_DIR}
            -DSRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/src
            -P ${CMAKE_CURRENT_SOURCE_DIR}/patch.cmake
        CONFIGURE_COMMAND ${PROJECT_BINARY_DIR}/src/configure --enable-optimizations --prefix=${installDir}
        BUILD_COMMAND make
        INSTALL_COMMAND
            rm -rf ${installDir} && mkdir ${installDir}
            COMMAND make install &&
            ${pipUpgrade}
    )
    
    ExternalProject_Add(${externalProjectCmd})

    add_dependencies(${PROJECT_NAME} ${SRKPROJ_TOOL_OPENSSL_NAME} ${SRKPROJ_TOOL_ZLIB_NAME})
endif ()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOLS_DIR_NAME})