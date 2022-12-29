srkSetSubProject("")

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_MEMORY_ALLOCATOR_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_MEMORY_ALLOCATOR_INCLUDE ${PROJECT_BINARY_DIR}/install/include)

include(ExternalProject)

srkCreateExternalProjectCmakeArgs(cmakeArgs)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.0.1
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
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND
            ${CMAKE_COMMAND}
            -DSRC_DIR=${PROJECT_BINARY_DIR}/src
            -DINSTALL_DIR=${PROJECT_BINARY_DIR}/install
            -P ${CMAKE_CURRENT_SOURCE_DIR}/install.cmake
    CMAKE_ARGS
        ${cmakeArgs}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})