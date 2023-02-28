srkSetSubProject("")

set(installDir ${PROJECT_BINARY_DIR}/install)

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_INCLUDE ${installDir}/include)

get_property(SRKPROJ_EXTERNAL_7Z_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_7Z_NAME")
get_property(SRKPROJ_EXTERNAL_7Z_HOST_EXE GLOBAL PROPERTY "SRKPROJ_EXTERNAL_7Z_HOST_EXE")

set(ver 1.3.239.0)
set(urlBase https://sdk.lunarg.com/sdk/download/${ver}/)
set(src nil)

if (SRK_OS_WINDOWS)
    set(dlName VulkanSDK-${ver}-Installer.exe)
    set(url ${urlBase}windows/${dlName})
    set(urlHash 312c6a7611ca8ae0796ee32cdc2e19c0cd18e5fd0efa0b64324c256e6647dff2)
    set(src ${PROJECT_BINARY_DIR}/src)

    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_LIB ${installDir}/lib/vulkan-1.lib)
elseif (SRK_OS_LINUX)
    set(dlName vulkansdk-linux-x86_64-${ver}.tar.gz)
    set(url ${urlBase}linux/${dlName})
    set(urlHash 1ac4e3b23f27570535026506fd9fcaf5ed56f8562c87cdd60afe7adf58b037e8)
    set(src ${PROJECT_BINARY_DIR}/src/${ver}/x86_64)

    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_SLIB ${installDir}/lib/libvku.a)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_LIB ${installDir}/lib/libvulkan.so)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_DLL ${installDir}/lib/libvulkan.so)
elseif (SRK_OS_MACOS)
    set(dlName vulkansdk-macos-${ver}.dmg)
    set(url ${urlBase}mac/${dlName})
    set(urlHash 60c5410e7daf5660e47b599b7a3b8870fc6441ed42b101329b1e28692c6d25da)
    set(src ${PROJECT_BINARY_DIR}/src/InstallVulkan.app/Contents/Resources/installer/macOS)

    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_LIB ${installDir}/lib/libvulkan.dylib)
    set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_DLL ${installDir}/lib/libvulkan.dylib)
endif ()

if (NOT (src STREQUAL nil))
    srkCheckExternalProjectVersion_DownloadFile(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/download/${dlName})

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