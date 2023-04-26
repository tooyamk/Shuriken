srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

set(installDir ${PROJECT_BINARY_DIR}/install)
set(binDir ${installDir}/bin)

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_INDENT_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_INDENT_BIN_DIR ${binDir})

get_property(SRKPROJ_TOOL_TEXINFO_NAME GLOBAL PROPERTY "SRKPROJ_TOOL_TEXINFO_NAME")
get_property(SRKPROJ_TOOL_TEXINFO_BIN_DIR GLOBAL PROPERTY "SRKPROJ_TOOL_TEXINFO_BIN_DIR")

set(fileName indent-2.2.13.tar.xz)
srkCheckExternalProjectVersion_DownloadFile(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/download/${fileName})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://ftp.gnu.org/gnu/indent/${fileName}
    DOWNLOAD_NO_EXTRACT ON
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND
        tar -xf ${PROJECT_BINARY_DIR}/download/${fileName} -C ${PROJECT_BINARY_DIR}/src --strip-components 1
        COMMAND ${PROJECT_BINARY_DIR}/src/configure --prefix=${installDir}
        COMMAND cp -r ${PROJECT_BINARY_DIR}/src/regression ${PROJECT_BINARY_DIR}/build
    BUILD_COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ${SRKPROJ_TOOL_TEXINFO_BIN_DIR}
    INSTALL_COMMAND make install
)

add_dependencies(${PROJECT_NAME} ${SRKPROJ_TOOL_TEXINFO_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOL_DIR_NAME})