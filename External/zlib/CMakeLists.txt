__set_project()
if (UNIX)
    set(__OUTPUT_NAME libz)
else ()
    set(__OUTPUT_NAME zlib)
endif ()
set(LIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/${__OUTPUT_NAME})
set(LIB_R ${LIB_PREFIX}${AE_LIB_SUFFIX})
set(LIB_D ${LIB_PREFIX}d${AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY PROJECT_ZLIB_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_ZLIB_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY PROJECT_ZLIB_LIB ${LIB_PREFIX}${AE_BUILD_TYPE_SUFFIX}${AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY PROJECT_ZLIB_LIB_R ${LIB_R})
set_property(GLOBAL PROPERTY PROJECT_ZLIB_LIB_D ${LIB_D})
set_property(GLOBAL PROPERTY PROJECT_ZLIB_DLL ${PROJECT_BINARY_DIR}/install/$<IF:$<PLATFORM_ID:Windows>,bin,lib>/${__OUTPUT_NAME}${AE_BUILD_TYPE_SUFFIX}${AE_DLL_SUFFIX})

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
    BINARY_DIR ${AE_BUILD_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND
        ${CMAKE_COMMAND}
        -DCMAKE_LISTS_FILE=${CMAKE_CURRENT_BINARY_DIR}/src/CMakeLists.txt
        -DAE_DLL_SUFFIX=${AE_DLL_SUFFIX}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch.cmake
    BUILD_BYPRODUCTS 
        ${LIB_R}
        ${LIB_D}
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=1
        ${__CMAKE_ARGS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${AE_EXTERNAL_FOLDER_NAME})