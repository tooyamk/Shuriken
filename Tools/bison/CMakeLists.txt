srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

set(installDir ${PROJECT_BINARY_DIR}/install)
set(binDir ${installDir}/bin)

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_BISON_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_BISON_DIR ${binDir})

get_property(SRKPROJ_TOOL_M4_NAME GLOBAL PROPERTY "SRKPROJ_TOOL_M4_NAME")
get_property(SRKPROJ_TOOL_M4_EXE GLOBAL PROPERTY "SRKPROJ_TOOL_M4_EXE")

set(fileName bison-3.8.tar.xz)
srkCheckExternalProjectVersion_DownloadFile(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/download/${fileName})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://ftp.gnu.org/gnu/bison/${fileName}
    DOWNLOAD_NO_EXTRACT ON
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND tar -xf ${PROJECT_BINARY_DIR}/download/${fileName} -C ${PROJECT_BINARY_DIR}/src --strip-components 1
    CONFIGURE_COMMAND ${PROJECT_BINARY_DIR}/src/configure M4=${SRKPROJ_TOOL_M4_EXE} --prefix=${installDir}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

add_dependencies(${PROJECT_NAME} ${SRKPROJ_TOOL_M4_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOL_DIR_NAME})