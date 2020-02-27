__set_project()
if (UNIX)
    set(__OUTPUT_NAME libz)
else ()
    set(__OUTPUT_NAME zlib)
endif ()
set(__LIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/${__OUTPUT_NAME})
set(__LIB_R ${__LIB_PREFIX}${__AE_LIB_SUFFIX})
set(__LIB_D ${__LIB_PREFIX}d${__AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_LIB ${__LIB_PREFIX}$<$<CONFIG:Debug>:d>${__AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_LIB_R ${__LIB_R})
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_LIB_D ${__LIB_D})
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_DLL ${PROJECT_BINARY_DIR}/install/$<IF:$<PLATFORM_ID:Windows>,bin,lib>/${__OUTPUT_NAME}$<$<CONFIG:Debug>:d>$<$<PLATFORM_ID:Windows>:1>${__AE_DLL_SUFFIX})

include(ExternalProject)

__create_external_project_cmake_args(__CMAKE_ARGS)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.2.11
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${__BUILD_DIR}
    UPDATE_COMMAND ""
    BUILD_BYPRODUCTS 
        ${__LIB_R}
        ${__LIB_D}
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=1
        ${__CMAKE_ARGS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_EXTERNAL_FOLDER_NAME})