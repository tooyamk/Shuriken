__set_project()

get_property(__PROJECT_AE_CORE_NAME GLOBAL PROPERTY "__PROJECT_AE_CORE_NAME")
get_property(__PROJECT_AE_CORE_SRC_DIR GLOBAL PROPERTY "__PROJECT_AE_CORE_SRC_DIR")

get_property(__PROJECT_LIBPNG_NAME GLOBAL PROPERTY "__PROJECT_LIBPNG_NAME")
get_property(__PROJECT_LIBPNG_INSTALL_DIR GLOBAL PROPERTY "__PROJECT_LIBPNG_INSTALL_DIR")

include_directories(${__PROJECT_AE_CORE_SRC_DIR}/src/)
include_directories(${__PROJECT_LIBPNG_INSTALL_DIR}/include/)
add_definitions(-D AE_EXTENSION_EXPORTS)
file(GLOB SRC_FILES "src/*.h" "src/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})
add_dependencies(${PROJECT_NAME} ${__PROJECT_AE_CORE_NAME})
add_dependencies(${PROJECT_NAME} ${__PROJECT_LIBPNG_NAME})
target_link_libraries(${PROJECT_NAME} ${__PROJECT_AE_CORE_NAME})
target_link_libraries(${PROJECT_NAME} ${__PROJECT_LIBPNG_INSTALL_DIR}/lib/libpng16$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_EXTENSIONS_FOLDER_NAME}/Files/Images)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ae-png-converter")