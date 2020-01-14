__set_project()
set(__OUTPUT_NAME ae-core)
set_property(GLOBAL PROPERTY __PROJECT_AE_CORE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_AE_CORE_SRC_DIR ${PROJECT_SOURCE_DIR})
set_property(GLOBAL PROPERTY __PROJECT_AE_CORE_DLL ${__BUILD_DIR}/${__OUTPUT_NAME}$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>${CMAKE_SHARED_LIBRARY_SUFFIX})

include_directories(src)
add_definitions(-D AE_EXPORTS)
file(GLOB SRC_FILES "src/*/*.h" "src/*/*.cpp" "src/*/*/*.h" "src/*/*/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${__OUTPUT_NAME})

#set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
#install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})