__set_project()
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_ZLIB_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    #URL https://www.zlib.net/zlib1211.zip
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
    #INSTALL_COMMAND ""
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=1
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
        _DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Libs")