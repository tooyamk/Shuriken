srkSetSubProject("")

set(installDir ${PROJECT_BINARY_DIR}/install)

set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_RT_NAME ${PROJECT_NAME})

set(ver 1.3.261.1)
set(urlBase https://sdk.lunarg.com/sdk/download/${ver}/)
set(src nil)

if (SRK_OS_WINDOWS AND SRK_ARCH_X86)
    set(dlName VulkanRT-${ver}-Components.zip)
    set(url ${urlBase}windows/${dlName})
    set(urlHash b77b5833ed0bc9463a1bc292a9337b4db719ab2359087c75c6f75ec4486bc794)
    set(src ${PROJECT_BINARY_DIR}/src)

    if (SRK_ARCH_WORD_BITS_32)
        set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_DLL ${installDir}/bin/x86/vulkan-1${SRK_DLL_SUFFIX})
    elseif (SRK_ARCH_WORD_BITS_64)
        set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_VULKAN_SDK_DLL ${installDir}/bin/x64/vulkan-1${SRK_DLL_SUFFIX})
    endif ()
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
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND
            ${CMAKE_COMMAND}
            -DSRC_DIR=${src}
            -DINSTALL_DIR=${installDir}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/install.cmake
    )
endif ()
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})