srkSetProject("")

include(ExternalProject)

srkCreateExternalProjectCmakeArgs(cmakeArgs)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/richgel999/bc7enc_rdo.git
    #GIT_TAG 3.7
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DISA_NONE=ON
        -DUNITTEST=OFF
        -DDIAGNOSTICS=OFF
        -DCLI=OFF
        -DCMAKE_INSTALL_PREFIX=../install
        ${args}
        ${cmakeArgs}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})