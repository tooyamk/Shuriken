auto_set_project_name()
set_property(GLOBAL PROPERTY __PROJECT_CORE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(src)
add_definitions(-D AE_EXPORTS)
file(GLOB SRC_FILES "src/*/*.h" "src/*/*.cpp" "src/*/*/*.h" "src/*/*/*.cpp")
add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})

#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build/)