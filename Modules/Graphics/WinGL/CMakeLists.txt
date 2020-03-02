__set_project()
set(__OUTPUT_NAME ae-win-gl)
set_property(GLOBAL PROPERTY __PROJECT_AE_WIN_GL_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_AE_WIN_GL_DLL ${__BUILD_DIR}/${__OUTPUT_NAME}${__BUILD_TYPE_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})

get_property(__PROJECT_AE_CORE_NAME GLOBAL PROPERTY "__PROJECT_AE_CORE_NAME")
get_property(__PROJECT_AE_CORE_SRC_DIR GLOBAL PROPERTY "__PROJECT_AE_CORE_SRC_DIR")

get_property(__PROJECT_GLEW_NAME GLOBAL PROPERTY "__PROJECT_GLEW_NAME")
get_property(__PROJECT_GLEW_INCLUDE GLOBAL PROPERTY "__PROJECT_GLEW_INCLUDE")
get_property(__PROJECT_GLEW_LIB GLOBAL PROPERTY "__PROJECT_GLEW_LIB")

include_directories(
    ${__PROJECT_AE_CORE_SRC_DIR}/src/
    ${__PROJECT_GLEW_INCLUDE}
)
add_definitions(-D AE_MODULE_EXPORTS)
file(GLOB SRC_FILES
    "src/*.h" "src/*.cpp"
    "src/*/*.h" "src/*/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})
add_dependencies(${PROJECT_NAME}
    ${__PROJECT_AE_CORE_NAME}
    ${__PROJECT_GLEW_NAME}
)

target_link_libraries(${PROJECT_NAME}
    ${__PROJECT_AE_CORE_NAME}
    ${__PROJECT_GLEW_LIB}
)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32.lib)
endif ()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_MODULES_FOLDER_NAME}/Graphics)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${__OUTPUT_NAME}) 