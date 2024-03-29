srkSetSubProject(${SRK_EXTENSION_PROJ_PREFIX})

set(OUTPUT_NAME ${SRK_EXTENSION_OUTPUT_NAME_PREFIX}shader-transpiler)
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_SHADER_TRANSPILER_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_SHADER_TRANSPILER_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/)
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_SHADER_TRANSPILER_OUTPUT_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTENSION_SHADER_TRANSPILER_DLL ${SRK_PROJECT_BUILD_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_DLL_SUFFIX})

get_property(SRKPROJ_CORE_NAME GLOBAL PROPERTY "SRKPROJ_CORE_NAME")
get_property(SRKPROJ_CORE_INCLUDE GLOBAL PROPERTY "SRKPROJ_CORE_INCLUDE")

get_property(SRKPROJ_FRAMEWORK_NAME GLOBAL PROPERTY "SRKPROJ_FRAMEWORK_NAME")
get_property(SRKPROJ_FRAMEWORK_INCLUDE GLOBAL PROPERTY "SRKPROJ_FRAMEWORK_INCLUDE")

get_property(SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_NAME")
get_property(SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_INCLUDE GLOBAL PROPERTY "SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_INCLUDE")

get_property(SRKPROJ_EXTERNAL_SPIRV_CROSS_NAME GLOBAL PROPERTY "SRKPROJ_EXTERNAL_SPIRV_CROSS_NAME")
get_property(SRKPROJ_EXTERNAL_SPIRV_CROSS_INCLUDE GLOBAL PROPERTY "SRKPROJ_EXTERNAL_SPIRV_CROSS_INCLUDE")

file(GLOB_RECURSE srcFiles "src/*.h" "src/*.cpp")
srkAddLibrarySourceGroup(${PROJECT_NAME} SHARED ${srcFiles})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${SRKPROJ_CORE_INCLUDE}
    ${SRKPROJ_FRAMEWORK_INCLUDE}
    ${SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_INCLUDE}
    ${SRKPROJ_EXTERNAL_SPIRV_CROSS_INCLUDE}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE SRK_EXT_SHD_TRANS_EXPORTS)
add_dependencies(${PROJECT_NAME}
    ${SRKPROJ_FRAMEWORK_NAME}
    ${SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_NAME}
    ${SRKPROJ_EXTERNAL_SPIRV_CROSS_NAME}
)
target_link_libraries(${PROJECT_NAME}
    ${SRKPROJ_CORE_NAME}
    ${SRKPROJ_FRAMEWORK_NAME}
)
#srkTargetLinkLibraryExternalProject(${PROJECT_NAME} "SRKPROJ_EXTERNAL_SPIRV_CROSS_LIB")
srkTargetLinkLibraryExternalProject(${PROJECT_NAME} "SRKPROJ_EXTERNAL_SPIRV_CROSS_CORE_SLIB")
srkTargetLinkLibraryExternalProject(${PROJECT_NAME} "SRKPROJ_EXTERNAL_SPIRV_CROSS_GLSL_SLIB")
srkTargetLinkLibraryExternalProject(${PROJECT_NAME} "SRKPROJ_EXTERNAL_SPIRV_CROSS_MSL_SLIB")
srkTargetLinkLibraryExternalProject(${PROJECT_NAME} "SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_LLVMDxcSupport_SLIB")
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTENSIONS_DIR_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
srkInstall(${SRK_EXTENSIONS_DIR_NAME}/${PROJECT_NAME})