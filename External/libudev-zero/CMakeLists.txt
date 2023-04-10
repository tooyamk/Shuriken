srkSetSubProject("")

set(OUTPUT_NAME libudev-zero-static)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_LIBUDEV_ZERO_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_LIBUDEV_ZERO_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_LIBUDEV_ZERO_SLIB ${PROJECT_BINARY_DIR}/install/lib/${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_SLIB_SUFFIX})

set(gitTag 1.0.1)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

srkCreateExternalProjectCmakeArgs(cmakeArgs)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/illiliti/libudev-zero.git
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
    PATCH_COMMAND
        ${CMAKE_COMMAND}
        -DPATCH_DIR=${CMAKE_CURRENT_SOURCE_DIR}/patch
        -DSRC_DIR=${PROJECT_BINARY_DIR}/src
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch/patch.cmake
    CMAKE_ARGS
        ${cmakeArgs}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})