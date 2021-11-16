__set_project()
set(__OUTPUT_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}glew)
set_property(GLOBAL PROPERTY PROJECT_GLEW_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_GLEW_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY PROJECT_GLEW_LIB ${PROJECT_BINARY_DIR}/install/lib/${__OUTPUT_NAME}${AE_BUILD_TYPE_SUFFIX}${AE_LIB_SUFFIX})
set_property(GLOBAL PROPERTY PROJECT_GLEW_DLL ${PROJECT_BINARY_DIR}/install/${AE_INSTALL_DLL_DIR_NAME}/${__OUTPUT_NAME}${AE_BUILD_TYPE_SUFFIX}${AE_DLL_SUFFIX})

include(ExternalProject)

#if (MSVC)
#   set(flags "/DYNAMICBASE \"msvcurt.lib\"")
#   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flags}")
#   set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${flags}")
#endif ()

__create_external_project_cmake_args(__CMAKE_ARGS)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    #URL https://nchc.dl.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0.zip
    GIT_REPOSITORY https://github.com/nigels-com/glew.git
    GIT_TAG glew-2.2.0
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
        -DGIT_ROOT=${PROJECT_BINARY_DIR}/src
        -DCMAKE_LISTS_FILE=${CMAKE_CURRENT_BINARY_DIR}/src/build/cmake/CMakeLists.txt
        -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}/src/
        -DDST_DIR=${CMAKE_CURRENT_BINARY_DIR}/src/
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch.cmake
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND}
        -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
        -DCMAKE_STATIC_LINKER_FLAGS=${CMAKE_STATIC_LINKER_FLAGS}
        ${__CMAKE_ARGS}
        <SOURCE_DIR>/build/cmake
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${AE_EXTERNAL_DIR_NAME})