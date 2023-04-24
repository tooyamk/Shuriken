srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

set(installDir ${PROJECT_BINARY_DIR}/install)
set(binDir ${installDir}/bin)

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_AUTOMAKE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_AUTOMAKE_ACLOCAL ${binDir}/aclocal)
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_AUTOMAKE_AUTOMAKE ${binDir}/automake)

get_property(SRKPROJ_TOOL_AUTOCONF_NAME GLOBAL PROPERTY "SRKPROJ_TOOL_AUTOCONF_NAME")
get_property(SRKPROJ_TOOL_AUTOCONF_DIR GLOBAL PROPERTY "SRKPROJ_TOOL_AUTOCONF_DIR")

set(fileName automake-1.16.5.tar.xz)
srkCheckExternalProjectVersion_DownloadFile(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/download/${fileName})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://ftp.gnu.org/gnu/automake/${fileName}
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND
        tar -xf ${PROJECT_BINARY_DIR}/download/${fileName} -C ${PROJECT_BINARY_DIR}/src --strip-components 1
        COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/configure.sh ${SRKPROJ_TOOL_AUTOCONF_DIR} ${PROJECT_BINARY_DIR}/src ${installDir}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

add_dependencies(${PROJECT_NAME} ${SRKPROJ_TOOL_AUTOCONF_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOL_DIR_NAME})