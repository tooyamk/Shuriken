cmake_minimum_required (VERSION 3.10.0)

set(CMAKE_DEBUG_POSTFIX d)
#set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "")
#set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "")
#set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "")
#set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "")

macro(__set_project)
    get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" ProjectId ${ProjectId})
    #project(${ProjectId})

    set(PROJECT_NAME ${ProjectId})
    set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    set(__BUILD_DIR ${PROJECT_BINARY_DIR}/build)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${__BUILD_DIR}$<0:>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${__BUILD_DIR}$<0:>)
    #set(LIBRARY_OUTPUT_PATH ${__BUILD_DIR}$<0:>)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${__BUILD_DIR}$<0:>)
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

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(Aurora)

set(__AE_EXTERNAL_FOLDER_NAME External)
set(__AE_MODULES_FOLDER_NAME Modules)
set(__AE_EXTENSIONS_FOLDER_NAME Extensions)

add_subdirectory(${__AE_EXTERNAL_FOLDER_NAME}/zlib)
add_subdirectory(${__AE_EXTERNAL_FOLDER_NAME}/libpng)
add_subdirectory(${__AE_EXTERNAL_FOLDER_NAME}/glew)
add_subdirectory(${__AE_EXTERNAL_FOLDER_NAME}/dxcompiler)#compile failed

add_subdirectory(Core)

if (WIN32)
    add_subdirectory(${__AE_MODULES_FOLDER_NAME}/Graphics/ProgramSourceTranslator)#only x64
    add_subdirectory(${__AE_MODULES_FOLDER_NAME}/Graphics/WinD3D11)
    add_subdirectory(${__AE_MODULES_FOLDER_NAME}/Graphics/WinGL)

    add_subdirectory(${__AE_MODULES_FOLDER_NAME}/Inputs/WinDirectInput)
    add_subdirectory(${__AE_MODULES_FOLDER_NAME}/Inputs/WinXInput)
endif ()

add_subdirectory(${__AE_EXTENSIONS_FOLDER_NAME}/Files/Images/PNGConverter)

add_subdirectory(Test)