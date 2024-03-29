srkSetSubProject(${SRK_MODULE_WINDOW_PROJ_PREFIX})

set(OUTPUT_NAME ${SRK_MODULE_WINDOW_OUTPUT_NAME_PREFIX}android-native)
set_property(GLOBAL PROPERTY SRKPROJ_MODULE_WINDOW_ANDROID_NATIVE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_MODULE_WINDOW_ANDROID_NATIVE_DLL ${SRK_PROJECT_BUILD_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_DLL_SUFFIX})

get_property(SRKPROJ_CORE_NAME GLOBAL PROPERTY "SRKPROJ_CORE_NAME")
get_property(SRKPROJ_CORE_INCLUDE GLOBAL PROPERTY "SRKPROJ_CORE_INCLUDE")

get_property(SRKPROJ_FRAMEWORK_NAME GLOBAL PROPERTY "SRKPROJ_FRAMEWORK_NAME")
get_property(SRKPROJ_FRAMEWORK_INCLUDE GLOBAL PROPERTY "SRKPROJ_FRAMEWORK_INCLUDE")

get_property(SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_NAME GLOBAL PROPERTY "SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_NAME")
get_property(SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_INCLUDE GLOBAL PROPERTY "SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_INCLUDE")

file(GLOB_RECURSE srcFiles "src/*.h" "src/*.cpp")
srkAddLibrarySourceGroup(${PROJECT_NAME} SHARED ${srcFiles})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${SRKPROJ_CORE_INCLUDE}
    ${SRKPROJ_FRAMEWORK_INCLUDE}
    ${SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_INCLUDE}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${SRK_MODULE_DEFINITIONS})
add_dependencies(${PROJECT_NAME} ${SRKPROJ_FRAMEWORK_NAME} ${SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_NAME})
target_link_libraries(${PROJECT_NAME}
    ${SRKPROJ_CORE_NAME}
    ${SRKPROJ_FRAMEWORK_NAME}
    ${SRKPROJ_EXTENSION_ANDROID_NATIVE_APPLICATION_NAME}
)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_MODULES_WINDOWS_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
srkInstall(${SRK_MODULES_WINDOWS_DIR}/${PROJECT_NAME})