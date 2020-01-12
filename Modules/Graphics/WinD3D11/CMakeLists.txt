__set_project()

get_property(__PROJECT_AE_CORE_NAME GLOBAL PROPERTY "__PROJECT_AE_CORE_NAME")
get_property(__PROJECT_AE_CORE_SRC_DIR GLOBAL PROPERTY "__PROJECT_AE_CORE_SRC_DIR")

include_directories(${__PROJECT_AE_CORE_SRC_DIR}/src/)
add_definitions(-D AE_MODULE_EXPORTS)
file(GLOB SRC_FILES "src/*.h" "src/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})
add_dependencies(${PROJECT_NAME} ${__PROJECT_AE_CORE_NAME})
target_link_libraries(${PROJECT_NAME} ${__PROJECT_AE_CORE_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_MODULES_FOLDER_NAME}/Graphics)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ae-win-d3d11)