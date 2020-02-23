__set_project()
set_property(GLOBAL PROPERTY __PROJECT_BOOST_PREPROCESSOR_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_BOOST_PREPROCESSOR_INCLUDE ${PROJECT_BINARY_DIR}/src/include)

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/boostorg/preprocessor.git
    GIT_TAG boost-1.72.0
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${__BUILD_DIR}
    UPDATE_COMMAND ""
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_EXTERNAL_FOLDER_NAME})