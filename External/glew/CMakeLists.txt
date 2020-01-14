__set_project()
set(__OUTPUT_NAME glew32)
set_property(GLOBAL PROPERTY __PROJECT_GLEW_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_GLEW_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY __PROJECT_GLEW_LIB ${PROJECT_BINARY_DIR}/install/lib/${__OUTPUT_NAME}$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(GLOBAL PROPERTY __PROJECT_GLEW_DLL ${PROJECT_BINARY_DIR}/install/bin/${__OUTPUT_NAME}$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL https://nchc.dl.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0.zip
    #GIT_REPOSITORY https://github.com/nigels-com/glew.git
    #GIT_TAG glew-2.1.0
    #GIT_SHALLOW 1
    #GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${__BUILD_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        <SOURCE_DIR>/build/cmake
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_EXTERNAL_FOLDER_NAME})