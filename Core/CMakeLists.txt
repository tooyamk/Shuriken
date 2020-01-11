__set_project()
set_property(GLOBAL PROPERTY __PROJECT_CORE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_CORE_SRC_DIR ${PROJECT_SOURCE_DIR})
set_property(GLOBAL PROPERTY __PROJECT_CORE_BUILD_DIR ${__BUILD_DIR})

message("=======================")
#message(${LIBRARY_OUTPUT_DIRECTORY})
#message(${LIBRARY_OUTPUT_DIRECTORY_DEBUG})

include_directories(src)
add_definitions(-D AE_EXPORTS)
file(GLOB SRC_FILES "src/*/*.h" "src/*/*.cpp" "src/*/*/*.h" "src/*/*/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})

#set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
#install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})