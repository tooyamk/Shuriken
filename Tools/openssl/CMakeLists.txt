srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

set(installDir ${PROJECT_BINARY_DIR}/install)
set(libDir ${installDir}/lib)
if (SRK_HOST_ARCH_WORD_BITS_64)
    set(libDir ${libDir}64)
endif ()

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_OPENSSL_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_OPENSSL_INCLUDE ${installDir}/include)
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_OPENSSL_LIB_DIR ${libDir})

set(gitTag openssl-3.1.0)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/openssl/openssl.git
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
    CONFIGURE_COMMAND COMMAND ${PROJECT_BINARY_DIR}/src/Configure --prefix=${installDir}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOLS_DIR_NAME})