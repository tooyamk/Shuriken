srkSetProject()

set(ver 1.3.216.0)
set(installDir ${PROJECT_BINARY_DIR}/install/)
set(verFile ${installDir}${ver}.txt)

set_property(GLOBAL PROPERTY PROJECT_VULKAN_SDK_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_VULKAN_SDK_INCLUDE ${installDir}include)

get_property(7Z_EXE GLOBAL PROPERTY "7Z_EXE")

if (NOT (EXISTS ${verFile}))
    file(REMOVE_RECURSE ${installDir})

    if (SRK_OS_WINDOWS)
        set(dlFileName VulkanSDK-${ver}-Installer.exe)
        set(dlFileHash 32c0bba765bd42d6e321fbb14a0abbb6d305da06f53db7d369a3cc295de111f8)
        set(dlFilePath ${PROJECT_BINARY_DIR}/download/${dlFileName})

        srkTryDownloadFile(https://sdk.lunarg.com/sdk/download/1.3.216.0/windows/${dlFileName} ${dlFilePath} ${dlFileHash})
        file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/src)
        execute_process(COMMAND ${7Z_EXE} x ${dlFilePath} -o${PROJECT_BINARY_DIR}/src)

        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/src/Bin ${installDir}bin)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/src/Include ${installDir}include)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/src/Lib ${installDir}lib)

        file(WRITE ${verFile})

        set_property(GLOBAL PROPERTY PROJECT_VULKAN_SDK_SLIB ${installDir}lib/vulkan-1.lib ${installDir}lib/VkLayer_utils.lib)
    endif ()
endif ()

add_custom_target(${PROJECT_NAME} ALL)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})