__set_project()
set(__OUTPUT_NAME ae-framework)
set_property(GLOBAL PROPERTY PROJECT_AE_FRAMEWORK_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_AE_FRAMEWORK_SRC_DIR ${PROJECT_SOURCE_DIR})
set_property(GLOBAL PROPERTY PROJECT_AE_FRAMEWORK_DLL ${AE_BUILD_DIR}/${__OUTPUT_NAME}${AE_BUILD_TYPE_SUFFIX}${AE_DLL_SUFFIX})

get_property(PROJECT_AE_CORE_NAME GLOBAL PROPERTY "PROJECT_AE_CORE_NAME")
get_property(PROJECT_AE_CORE_SRC_DIR GLOBAL PROPERTY "PROJECT_AE_CORE_SRC_DIR")

include_directories(
    src
    ${PROJECT_AE_CORE_SRC_DIR}/src/
)
add_definitions(-DAE_FW_EXPORTS)
file(GLOB SRC_FILES
    "src/*.h" "src/*.cpp"
    "src/*/*.h" "src/*/*.cpp"
    "src/*/*/*.h" "src/*/*/*.cpp"
    "src/*/*/*/*.h" "src/*/*/*/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})
add_dependencies(${PROJECT_NAME} 
    ${PROJECT_AE_CORE_NAME}
)
target_link_libraries(${PROJECT_NAME}
    ${PROJECT_AE_CORE_NAME}
)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${__OUTPUT_NAME})
__install(${PROJECT_NAME})

#set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
#install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})