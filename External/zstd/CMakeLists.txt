srkSetSubProject("")

set(OUTPUT_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}zstd)
set(LIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/${OUTPUT_NAME})
set(LIB_R ${LIB_PREFIX}${CMAKE_RELEASE_POSTFIX}${SRK_LIB_SUFFIX})
set(LIB_D ${LIB_PREFIX}${CMAKE_DEBUG_POSTFIX}${SRK_LIB_SUFFIX})

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_ZSTD_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_ZSTD_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_ZSTD_LIB ${LIB_PREFIX}${SRK_BUILD_TYPE_SUFFIX}${SRK_LIB_SUFFIX})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_ZSTD_LIB_R ${LIB_R})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_ZSTD_LIB_D ${LIB_D})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_ZSTD_DLL ${PROJECT_BINARY_DIR}/install/${SRK_INSTALL_DLL_DIR_NAME}/${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_DLL_SUFFIX})

set(gitTag v1.5.5)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

srkCreateExternalProjectCmakeArgs(cmakeArgs)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/facebook/zstd.git
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
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch.cmake
    BUILD_BYPRODUCTS 
        ${LIB_R}
        ${LIB_D}
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND}
        -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
        -DCMAKE_STATIC_LINKER_FLAGS=${CMAKE_STATIC_LINKER_FLAGS}
        -DZSTD_BUILD_PROGRAMS=OFF
        -DZSTD_BUILD_TESTS=OFF
        -DZSTD_BUILD_STATIC=OFF
        ${cmakeArgs}
        <SOURCE_DIR>/build/cmake
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})