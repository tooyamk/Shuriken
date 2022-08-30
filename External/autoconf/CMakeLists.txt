srkSetSubProject("")

set(installDir ${PROJECT_BINARY_DIR}/install)
set(binDir ${installDir}/bin)

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_DIR ${binDir})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_AUTOCONF ${binDir}/autoconf)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_AUTOHEADER ${binDir}/autoheader)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_AUTOM4TE ${binDir}/autom4te)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_AUTORECONF ${binDir}/autoreconf)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_AUTOCONF_AUTOUPDATE ${binDir}/autoupdate)

get_property(SRKPROJ_EXTERNAL_M4_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_M4_NAME")
get_property(SRKPROJ_EXTERNAL_M4_EXE GLOBAL PROPERTY "SRKPROJ_EXTERNAL_M4_EXE")

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://ftp.gnu.org/gnu/autoconf/autoconf-2.71.tar.xz
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND
        ${COMMAND}
        ${PROJECT_BINARY_DIR}/src/configure
        M4=${SRKPROJ_EXTERNAL_M4_EXE}
        --prefix=${installDir}
    BUILD_COMMAND
        ${COMMAND}
        make
    INSTALL_COMMAND
        ${COMMAND}
        make install
)

add_dependencies(${PROJECT_NAME} ${SRKPROJ_EXTERNAL_M4_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})