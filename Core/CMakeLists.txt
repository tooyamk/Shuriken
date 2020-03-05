__set_project()
set(__OUTPUT_NAME ae-core)
set_property(GLOBAL PROPERTY PROJECT_AE_CORE_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_AE_CORE_SRC_DIR ${PROJECT_SOURCE_DIR})
set_property(GLOBAL PROPERTY PROJECT_AE_CORE_DLL ${AE_BUILD_DIR}/${__OUTPUT_NAME}${AE_BUILD_TYPE_POSTFIX}${AE_DLL_SUFFIX})

include_directories(src)
add_definitions(-DAE_EXPORTS)
file(GLOB SRC_FILES
    "src/*.h" "src/*.cpp"
    "src/*/*.h" "src/*/*.cpp"
    "src/*/*/*.h" "src/*/*/*.cpp"
    "src/*/*/*/*.h" "src/*/*/*/*.cpp")
__add_library_source_group(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${__OUTPUT_NAME})

#set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
#install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})