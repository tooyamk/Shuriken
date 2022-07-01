srkSetProject()

set(installDir ${PROJECT_BINARY_DIR}/install)

set_property(GLOBAL PROPERTY PROJECT_VULKAN_SDK_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_VULKAN_SDK_INCLUDE ${installDir}/include)

get_property(7Z_EXE GLOBAL PROPERTY "7Z_EXE")

if (SRK_OS_WINDOWS)
    set(dlName VulkanSDK-1.3.216.0-Installer.exe)
    set(url https://sdk.lunarg.com/sdk/download/1.3.216.0/windows/${dlName})
    set(urlHash 32c0bba765bd42d6e321fbb14a0abbb6d305da06f53db7d369a3cc295de111f8)

    set_property(GLOBAL PROPERTY PROJECT_VULKAN_SDK_SLIB ${installDir}/lib/vulkan-1.lib ${installDir}/lib/VkLayer_utils.lib)
endif ()

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    URL ${url}
    URL_HASH SHA256=${urlHash}
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_BUILD_DIR}
    UPDATE_COMMAND ""
    DOWNLOAD_NO_EXTRACT True
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND}
        -DSRC_DIR=${PROJECT_BINARY_DIR}/src
        -DFILE_PATH=${PROJECT_BINARY_DIR}/download/${dlName}
        -DSRK_OS_WINDOWS=${SRK_OS_WINDOWS}
        -D7Z_EXE=${7Z_EXE}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake
    BUILD_COMMAND ""
    INSTALL_COMMAND
        ${CMAKE_COMMAND}
        -DSRC_DIR=${PROJECT_BINARY_DIR}/src
        -DINSTALL_DIR=${installDir}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/install.cmake
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})