cmake_minimum_required (VERSION 3.10.0)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "")#rd)
set(CMAKE_MINSIZEREL_POSTFIX "")#rs)
set(AE_BUILD_TYPE_SUFFIX $<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>$<$<CONFIG:Release>:${CMAKE_RELEASE_POSTFIX}>$<$<CONFIG:RelWithDebInfo>:${CMAKE_RELWITHDEBINFO_POSTFIX}>$<$<CONFIG:MinSizeRel>:${CMAKE_MINSIZEREL_POSTFIX}>)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
#set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "")
#set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "")
#set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "")
#set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "")

option(AE_TESTS "" OFF)

macro(__set_project)
    get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" ProjectId ${ProjectId})
    #project(${ProjectId})

    set(PROJECT_NAME ${ProjectId})
    set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    set(AE_BUILD_DIR ${PROJECT_BINARY_DIR}/build)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${AE_BUILD_DIR}$<0:>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${AE_BUILD_DIR}$<0:>)
    #set(LIBRARY_OUTPUT_PATH ${AE_BUILD_DIR}$<0:>)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AE_BUILD_DIR}$<0:>)
endmacro()

function(__assign_source_group)
    foreach (_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else ()
            set(_source_rel "${_source}")
        endif ()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach ()
endfunction(__assign_source_group)
 
function(__add_library_source_group)
    foreach (_source IN ITEMS ${ARGN})
        __assign_source_group(${_source})
    endforeach ()
    add_library(${ARGV})
endfunction(__add_library_source_group)

function(__add_executable_source_group)
    foreach (_source IN ITEMS ${ARGN})
        __assign_source_group(${_source})
    endforeach ()
    add_executable(${ARGV})
endfunction(__add_executable_source_group)

function(__target_link_library_external_project target lib_prefix)
    set(tmp_dumy_lib tmp_dumy_lib_${target}_${lib_prefix})
    add_library(${tmp_dumy_lib} UNKNOWN IMPORTED)
    get_property(tmp_var GLOBAL PROPERTY ${lib_prefix}_R)
    if (tmp_var)
        set_property(TARGET ${tmp_dumy_lib} PROPERTY IMPORTED_LOCATION ${tmp_var})
    endif()
    get_property(tmp_var GLOBAL PROPERTY ${lib_prefix}_D)
    if (tmp_var)
        set_property(TARGET ${tmp_dumy_lib} PROPERTY IMPORTED_LOCATION_DEBUG ${tmp_var})
    endif()
    target_link_libraries(${target} ${tmp_dumy_lib})
    unset(tmp_var)
    unset(tmp_dumy_lib)
endfunction(__target_link_library_external_project)

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_C_COMPILER_ID MATCHES Clang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fms-extensions")
endif ()
if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
endif ()

project(Aurora)

#cmake_policy(SET CMP0060 NEW)

if (WIN32)
    set(AE_SLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(AE_LIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(AE_DLL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else ()
    set(AE_SLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(AE_LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(AE_DLL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif ()

macro(__create_external_project_cmake_args target)
    set(${target}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        #-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        -DANDROID_ABI=${ANDROID_ABI}
        -DANDROID_PLATFORM=${ANDROID_PLATFORM}
        -DANDROID_NDK=${ANDROID_NDK}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
    )
endmacro()

set(AE_EXTERNAL_FOLDER_NAME External)
set(AE_MODULES_FOLDER_NAME Modules)
set(AE_EXTENSIONS_FOLDER_NAME Extensions)

SET(AE_ROOT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${AE_EXTERNAL_FOLDER_NAME}/zlib)
add_subdirectory(${AE_EXTERNAL_FOLDER_NAME}/libpng)
add_subdirectory(${AE_EXTERNAL_FOLDER_NAME}/glew)
add_subdirectory(${AE_EXTERNAL_FOLDER_NAME}/DirectXShaderCompiler)
add_subdirectory(${AE_EXTERNAL_FOLDER_NAME}/SPIRV-Cross)
add_subdirectory(${AE_EXTERNAL_FOLDER_NAME}/BoostPreprocessor)

add_subdirectory(Core)

add_subdirectory(${AE_MODULES_FOLDER_NAME}/Graphics/ProgramSourceTranslator)#??only x64
if (WIN32)
    add_subdirectory(${AE_MODULES_FOLDER_NAME}/Graphics/WinD3D11)
    add_subdirectory(${AE_MODULES_FOLDER_NAME}/Graphics/WinGL)

    add_subdirectory(${AE_MODULES_FOLDER_NAME}/Inputs/WinDirectInput)
    add_subdirectory(${AE_MODULES_FOLDER_NAME}/Inputs/WinXInput)
endif ()

add_subdirectory(${AE_EXTENSIONS_FOLDER_NAME}/PNGConverter)
add_subdirectory(${AE_EXTENSIONS_FOLDER_NAME}/FBXConverter)
add_subdirectory(${AE_EXTENSIONS_FOLDER_NAME}/ShaderScript)

if (AE_TESTS)
    add_subdirectory(Test)
endif ()