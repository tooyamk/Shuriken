srkSetSubProject("")

set(installDir ${PROJECT_BINARY_DIR}/install)

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_INCLUDE ${installDir}/include)

get_property(SRKPROJ_EXTERNAL_7Z_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_7Z_NAME")
get_property(SRKPROJ_EXTERNAL_7Z_HOST_EXE GLOBAL PROPERTY "SRKPROJ_EXTERNAL_7Z_HOST_EXE")

set(ver 1.3.236.0)
set(urlBase https://sdk.lunarg.com/sdk/download/${ver}/)
set(src nil)

if (SRK_OS_WINDOWS)
    set(dlName VulkanSDK-${ver}-Installer.exe)
    set(url ${urlBase}windows/${dlName})
    set(urlHash 0b21c602ef0d62ee49c39e03b35207d136130bf110682d7cad2c620265031dcc)
    set(src ${PROJECT_BINARY_DIR}/src)

    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_LIB ${installDir}/lib/vulkan-1.lib)
elseif (SRK_OS_LINUX)
    set(dlName vulkansdk-linux-x86_64-${ver}.tar.gz)
    set(url ${urlBase}linux/${dlName})
    set(urlHash 92a44158004a05fd3fba18963eebe3698442c93488e94c126361851a3ea002cf)
    set(src ${PROJECT_BINARY_DIR}/src/${ver}/x86_64)

    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_SLIB ${installDir}/lib/libvku.a)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_LIB ${installDir}/lib/libvulkan.so)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_DLL ${installDir}/lib/libvulkan.so)
elseif (SRK_OS_MACOS)
    set(dlName vulkansdk-macos-${ver}.dmg)
    set(url ${urlBase}mac/${dlName})
    set(urlHash 18b138c3fd91d5d457ec60bf264296b04332715bb379babc90c6e9c4fa45086c)
    set(src ${PROJECT_BINARY_DIR}/src/InstallVulkan.app/Contents/Resources/installer/macOS)

    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_LIB ${installDir}/lib/libvulkan.dylib)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_DLL ${installDir}/lib/libvulkan.dylib)
endif ()

if (NOT (src STREQUAL nil))
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
        BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
        UPDATE_COMMAND ""
        DOWNLOAD_NO_EXTRACT True
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND}
            -DSRC_DIR=${PROJECT_BINARY_DIR}/src
            -DFILE_PATH=${PROJECT_BINARY_DIR}/download/${dlName}
            -DSRK_OS_WINDOWS=${SRK_OS_WINDOWS}
            -DSRK_OS_LINUX=${SRK_OS_LINUX}
            -DSRK_OS_MACOS=${SRK_OS_MACOS}
            -D7Z_EXE=${SRKPROJ_EXTERNAL_7Z_HOST_EXE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake
        BUILD_COMMAND ""
        INSTALL_COMMAND
            ${CMAKE_COMMAND}
            -DSRC_DIR=${src}
            -DINSTALL_DIR=${installDir}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/install.cmake
    )

    if (SRK_OS_WINDOWS OR SRK_OS_MACOS)
        add_dependencies(${PROJECT_NAME} ${SRKPROJ_EXTERNAL_7Z_NAME})
    endif ()
endif ()
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})