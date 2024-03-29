srkSetSubProject(${SRK_EXTENSION_PROJ_PREFIX})

set(OUTPUT_NAME ${SRK_EXTENSION_OUTPUT_NAME_PREFIX}jpeg-converter)
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_JPEG_CONVERTER_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_JPEG_CONVERTER_INCLUDE ${PROJECT_SOURCE_DIR}/src/)
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_JPEG_CONVERTER_OUTPUT_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_JPEG_CONVERTER_DLL ${SRK_PROJECT_BUILD_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_DLL_SUFFIX})

get_property(SRKPROJ_CORE_NAME GLOBAL PROPERTY "SRKPROJ_CORE_NAME")
get_property(SRKPROJ_CORE_INCLUDE GLOBAL PROPERTY "SRKPROJ_CORE_INCLUDE")

get_property(SRKPROJ_FRAMEWORK_NAME GLOBAL PROPERTY "SRKPROJ_FRAMEWORK_NAME")
get_property(SRKPROJ_FRAMEWORK_INCLUDE GLOBAL PROPERTY "SRKPROJ_FRAMEWORK_INCLUDE")

get_property(SRKPROJ_EXTERNAL_LIBJPEG_TURBO_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_LIBJPEG_TURBO_NAME")
get_property(SRKPROJ_EXTERNAL_LIBJPEG_TURBO_INCLUDE GLOBAL PROPERTY "SRKPROJ_EXTERNAL_LIBJPEG_TURBO_INCLUDE")

file(GLOB_RECURSE srcFiles "src/*.h" "src/*.cpp")
srkAddLibrarySourceGroup(${PROJECT_NAME} SHARED ${srcFiles})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${SRKPROJ_CORE_INCLUDE}
    ${SRKPROJ_FRAMEWORK_INCLUDE}
    ${SRKPROJ_EXTERNAL_LIBJPEG_TURBO_INCLUDE}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE SRK_EXT_JPG_CONV_EXPORTS)
add_dependencies(${PROJECT_NAME} 
    ${SRKPROJ_FRAMEWORK_NAME}
    ${SRKPROJ_EXTERNAL_LIBJPEG_TURBO_NAME}
)
target_link_libraries(${PROJECT_NAME}
    ${SRKPROJ_CORE_NAME}
    ${SRKPROJ_FRAMEWORK_NAME}
)
srkTargetLinkLibraryExternalProject(${PROJECT_NAME} "SRKPROJ_EXTERNAL_LIBJPEG_TURBO_LIB")
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTENSIONS_DIR_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
srkInstall(${SRK_EXTENSIONS_DIR_NAME}/${PROJECT_NAME})