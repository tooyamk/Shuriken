srkSetSubProject("")

set(installDir ${PROJECT_BINARY_DIR}/install)
set(binDir ${installDir}/bin)

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOMAKE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOMAKE_ACLOCAL ${binDir}/aclocal)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOMAKE_AUTOMAKE ${binDir}/automake)

get_property(SRKPROJ_EXTERNAL_AUTOCONF_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_AUTOCONF_NAME")
get_property(SRKPROJ_EXTERNAL_AUTOCONF_DIR GLOBAL PROPERTY "SRKPROJ_EXTERNAL_AUTOCONF_DIR")

set(fileName automake-1.16.tar.xz)
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
        ${COMMAND}
        sh
        ${CMAKE_CURRENT_SOURCE_DIR}/configure.sh
        ${SRKPROJ_EXTERNAL_AUTOCONF_DIR}
        ${PROJECT_BINARY_DIR}/src
        ${installDir}
    BUILD_COMMAND
        ${COMMAND}
        make
    INSTALL_COMMAND
        ${COMMAND}
        make install
)

add_dependencies(${PROJECT_NAME} ${SRKPROJ_EXTERNAL_AUTOCONF_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})