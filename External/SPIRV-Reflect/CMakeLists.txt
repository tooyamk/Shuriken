srkSetSubProject("")

set(SLIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}spirv-reflect-static)
set(SLIB_R ${SLIB_PREFIX}${CMAKE_RELEASE_POSTFIX}${SRK_SLIB_SUFFIX})
set(SLIB_D ${SLIB_PREFIX}${CMAKE_DEBUG_POSTFIX}${SRK_SLIB_SUFFIX})

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_SPIRV_REFLECT_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_SPIRV_REFLECT_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_SPIRV_REFLECT_SLIB ${SLIB_PREFIX}${SRK_BUILD_TYPE_SUFFIX}${SRK_SLIB_SUFFIX})

set(gitTag sdk-1.3.243.0)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

srkCreateExternalProjectCmakeArgs(cmakeArgs)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
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
        -DSRC_DIR=${PROJECT_BINARY_DIR}/src
        -DINSTALL_DIR=${PROJECT_BINARY_DIR}/install
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch.cmake
    BUILD_BYPRODUCTS
        ${SLIB_R}
        ${SLIB_D}
    CMAKE_ARGS
        -DSPIRV_REFLECT_EXECUTABLE=OFF
        -DSPIRV_REFLECT_EXAMPLES=OFF
        -DSPIRV_REFLECT_STATIC_LIB=ON
        ${cmakeArgs}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})