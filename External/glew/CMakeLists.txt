srkSetProject("")
set(OUTPUT_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}glew)
set_property(GLOBAL PROPERTY SRKPROJ_GLEW_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_GLEW_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY SRKPROJ_GLEW_LIB ${PROJECT_BINARY_DIR}/install/lib/${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_LIB_SUFFIX})
set_property(GLOBAL PROPERTY SRKPROJ_GLEW_DLL ${PROJECT_BINARY_DIR}/install/${SRK_INSTALL_DLL_DIR_NAME}/${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_DLL_SUFFIX})

include(ExternalProject)

#if (SRK_CXX_COMPILER_MSVC)
#   set(flags "/DYNAMICBASE \"msvcurt.lib\"")
#   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flags}")
#   set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${flags}")
#endif ()

srkCreateExternalProjectCmakeArgs(cmakeArgs)

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
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND
        ${CMAKE_COMMAND}
        -DSRC_DIR=${PROJECT_BINARY_DIR}/src
        -DPATCH_DIR=${CMAKE_CURRENT_SOURCE_DIR}/patch
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch/patch.cmake
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND}
        -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
        -DCMAKE_STATIC_LINKER_FLAGS=${CMAKE_STATIC_LINKER_FLAGS}
        ${cmakeArgs}
        <SOURCE_DIR>/build/cmake
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})