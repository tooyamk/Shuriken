__set_project()
__set_output_name(__OUTPUT_NAME mimalloc)
set(LIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/mimalloc-2.0/${__OUTPUT_NAME})
set(LIB_R ${LIB_PREFIX}${AE_LIB_SUFFIX})
set(LIB_D ${LIB_PREFIX}d${AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY PROJECT_MIMALLOC_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_MIMALLOC_INCLUDE ${PROJECT_BINARY_DIR}/install/lib/mimalloc-2.0/include)
set_property(GLOBAL PROPERTY PROJECT_MIMALLOC_LIB ${LIB_PREFIX}${AE_BUILD_TYPE_SUFFIX}${AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY PROJECT_MIMALLOC_LIB_R ${LIB_R})
set_property(GLOBAL PROPERTY PROJECT_MIMALLOC_LIB_D ${LIB_D})
set_property(GLOBAL PROPERTY PROJECT_MIMALLOC_DLL ${PROJECT_BINARY_DIR}/install/lib/mimalloc-2.0/${__OUTPUT_NAME}${AE_BUILD_TYPE_SUFFIX}${AE_DLL_SUFFIX})

include(ExternalProject)

__create_external_project_cmake_args(__CMAKE_ARGS)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG v2.0.2
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${AE_BUILD_DIR}
    UPDATE_COMMAND ""
    BUILD_BYPRODUCTS 
        ${LIB_R}
        ${LIB_D}
    CMAKE_ARGS
        -DMI_BUILD_TESTS=OFF
        -DMI_BUILD_OBJECT=OFF
        ${__CMAKE_ARGS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${AE_EXTERNAL_DIR_NAME})