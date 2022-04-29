cmake_minimum_required (VERSION 3.15.0)

set(AE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
project(Aurora)
set(CMAKE_INSTALL_PREFIX ${AE_INSTALL_PREFIX})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Tools/cmake/Modules")

find_package(OpenGL)
find_package(UDev)
find_package(Vulkan)
find_package(X11)

macro(__set_project)
    get_filename_component(_projectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" _projectId ${_projectId})
    #project(${_projectId})

    set(PROJECT_NAME ${_projectId})
    set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    set(AE_BUILD_DIR ${PROJECT_BINARY_DIR}/build/)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${AE_BUILD_DIR}$<0:>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${AE_BUILD_DIR}$<0:>)
    #set(LIBRARY_OUTPUT_PATH ${AE_BUILD_DIR}$<0:>)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AE_BUILD_DIR}$<0:>)
    
    unset(_projectId)
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
endfunction()
 
function(__add_library_source_group)
    foreach (_source IN ITEMS ${ARGN})
        __assign_source_group(${_source})
    endforeach ()
    add_library(${ARGV})
endfunction()

function(__add_executable_source_group)
    foreach (_source IN ITEMS ${ARGN})
        __assign_source_group(${_source})
    endforeach ()
    add_executable(${ARGV})
endfunction()

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
endfunction()

function(__sub_dir_list cur dest)
    file(GLOB children RELATIVE ${cur} ${cur}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${cur}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${dest} ${dirlist} PARENT_SCOPE)
endfunction()

function(__install name)
    if (AE_INSTALL)
        install(DIRECTORY "src/" DESTINATION ${name}/include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*impl*.*" EXCLUDE)
        install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION ${name}/bin
            ARCHIVE DESTINATION ${name}/lib
            LIBRARY DESTINATION ${name}/lib
        )
    endif ()
endfunction()

function(__append_if cond arr value)
    if (${cond})
        list(APPEND arr ${value})
        set(${arr} ${${arr}} PARENT_SCOPE)
    endif ()
endfunction()

function(__append_copy_file_cmd cmd src dst)
    list(APPEND ${cmd} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst})
    set(${cmd} ${${cmd}} PARENT_SCOPE)
endfunction()

function(__append_copy_file_cmd_if cond cmd src dst)
    if (${cond})
        list(APPEND ${cmd} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst})
        set(${cmd} ${${cmd}} PARENT_SCOPE)
    endif ()
endfunction()

function(__add_subdirectory_if cond dir)
    if (${cond})
        add_subdirectory(${dir})
    endif ()
endfunction()

option(ASTC_ENCODER "" OFF)
option(DX_SHADER_COMPILER "" OFF)
option(GLEW "" OFF)
option(LIBPNG "" OFF)
option(SPIRV_CROSS "" OFF)
option(ZLIB "" OFF)
option(MIMALLOC "" OFF)
option(TCMALLOC "" OFF)

option(AE_CORE "" ON)
option(AE_FRAMEWORK "" ON)

option(AE_ALL_MODULES "" OFF)

option(AE_SHADER_TRANSPILER "" OFF)
option(AE_GRAPHICS_D3D11 "" ON)
option(AE_GRAPHICS_GL "" ON)
option(AE_GRAPHICS_VULKAN "" ON)

option(AE_INPUT_HID_INPUT "" OFF)
option(AE_INPUT_DIRECT_INPUT "" OFF)
option(AE_INPUT_RAW_INPUT "" OFF)
option(AE_INPUT_XINPUT "" OFF)

option(AE_ALL_EXTENSIONS "" OFF)

option(AE_ASTC_CONVERTER "" OFF)
option(AE_FBX_CONVERTER "" OFF)
option(AE_HID "" OFF)
option(AE_PNG_CONVERTER "" OFF)
option(AE_SHADER_SCRIPT "" OFF)

option(AE_TESTS "" OFF)

option(AE_INSTALL "" ON)

if (AE_TESTS)
    set(AE_ALL_MODULES ON)
    set(AE_ALL_EXTENSIONS ON)
    set(MIMALLOC ON)
    set(TCMALLOC ON)
endif ()

if (AE_ALL_MODULES)
    set(AE_SHADER_TRANSPILER ON)
    set(AE_GRAPHICS_D3D11 ON)
    set(AE_GRAPHICS_GL ON)
    set(AE_GRAPHICS_VULKAN ON)

    set(AE_INPUT_DIRECT_INPUT ON)
    set(AE_INPUT_HID_INPUT ON)
    set(AE_INPUT_RAW_INPUT ON)
    set(AE_INPUT_XINPUT ON)
endif ()

if (AE_ALL_EXTENSIONS)
    set(AE_ASTC_CONVERTER ON)
    set(AE_FBX_CONVERTER ON)
    set(AE_HID ON)
    set(AE_PNG_CONVERTER ON)
    set(AE_SHADER_SCRIPT ON)
endif ()

#validity check
#==========
if (NOT WIN32)
    set(AE_GRAPHICS_D3D11 OFF)

    set(AE_INPUT_DIRECT_INPUT OFF)
    set(AE_INPUT_RAW_INPUT OFF)
    set(AE_INPUT_XINPUT OFF)
endif ()

if (UNIX)
    if (AE_HID OR AE_INPUT_HID_INPUT)
        if (NOT UDEV_FOUND)
            set(AE_HID OFF)
            set(AE_INPUT_HID_INPUT OFF)
        endif ()
    endif ()
endif ()

if (GLEW OR AE_GRAPHICS_GL)
    if (NOT OPENGL_FOUND)
        set(GLEW OFF)
        set(AE_GRAPHICS_GL OFF)
    endif ()
endif ()

if (AE_GRAPHICS_VULKAN)
    if (NOT Vulkan_FOUND)
        set(AE_GRAPHICS_VULKAN OFF)
    endif ()
endif ()
#==========

#dependence check
#==========
if (AE_SHADER_SCRIPT)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_ASTC_CONVERTER)
    set(ASTC_ENCODER ON)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_FBX_CONVERTER)
    set(ZLIB ON)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_INPUT_HID_INPUT)
    set(AE_FRAMEWORK ON)
    set(AE_HID ON)
endif ()

if (AE_PNG_CONVERTER)
    set(LIBPNG ON)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_INPUT_DIRECT_INPUT)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_INPUT_RAW_INPUT)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_INPUT_XINPUT)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_GRAPHICS_D3D11)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_GRAPHICS_GL)
    set(GLEW ON)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_GRAPHICS_VULKAN)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_SHADER_TRANSPILER)
    set(DX_SHADER_COMPILER ON)
    set(SPIRV_CROSS ON)
endif ()

if (AE_FRAMEWORK)
    set(AE_CORE ON)
endif ()

if (LIBPNG)
    set(ZLIB ON)
endif ()
#==========

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "")#rd)
set(CMAKE_MINSIZEREL_POSTFIX "")#rs)
set(AE_BUILD_TYPE_SUFFIX $<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>$<$<CONFIG:Release>:${CMAKE_RELEASE_POSTFIX}>$<$<CONFIG:RelWithDebInfo>:${CMAKE_RELWITHDEBINFO_POSTFIX}>$<$<CONFIG:MinSizeRel>:${CMAKE_MINSIZEREL_POSTFIX}>)
set(AE_INSTALL_DLL_DIR_NAME $<IF:$<PLATFORM_ID:Windows>,bin,lib>)

#set(CMAKE_SHARED_LIBRARY_PREFIX "")
#set(CMAKE_STATIC_LIBRARY_PREFIX "")
#set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "")
#set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "")
#set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "")
#set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "")

set(CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(compilerFlags "-fms-extensions -fPIC -Wno-unused-command-line-argument -Wno-switch")
if (CMAKE_C_COMPILER_ID MATCHES "(Clang)|(GNU)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compilerFlags}")
endif ()
if (CMAKE_CXX_COMPILER_ID MATCHES "(Clang)|(GNU)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compilerFlags}")
endif ()

set(compilerFlags " /wd4244 /wd4251 /wd4267 /wd4275 /wd4819 /wd4838 /wd4996 /wd26812")
if (CMAKE_C_COMPILER_ID MATCHES MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compilerFlags}")
endif ()
if (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compilerFlags}")
endif ()

unset(compilerFlags)
#cmake_policy(SET CMP0060 NEW)

set(AE_DEFINITIONS)
if (CMAKE_BUILD_TYPE MATCHES "Deb")
    LIST(APPEND AE_DEFINITIONS -DAE_DEBUG)
endif ()

set(AE_MODULE_DEFINITIONS ${AE_DEFINITIONS} -DAE_MODULE_EXPORTS)
set(AE_EXTENSION_DEFINITIONS ${AE_DEFINITIONS} -DAE_EXTENSION_EXPORTS)

if (WIN32)
    set(AE_SLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(AE_LIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(AE_DLL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else ()
    set(AE_SLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(AE_LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(AE_DLL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif ()

set(AE_EXTERNAL_DIR_NAME External)
set(AE_MODULES_DIR_NAME Modules)
set(AE_GRAPHICS_MODULES_DIR ${AE_MODULES_DIR_NAME}/Graphics)
set(AE_INPUT_MODULES_DIR ${AE_MODULES_DIR_NAME}/Inputs)
set(AE_EXTENSIONS_DIR_NAME Extensions)

set(AE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

__add_subdirectory_if(ASTC_ENCODER ${AE_EXTERNAL_DIR_NAME}/astc-encoder)
__add_subdirectory_if(DX_SHADER_COMPILER ${AE_EXTERNAL_DIR_NAME}/DirectXShaderCompiler)
__add_subdirectory_if(GLEW ${AE_EXTERNAL_DIR_NAME}/glew)
__add_subdirectory_if(ZLIB ${AE_EXTERNAL_DIR_NAME}/zlib)
__add_subdirectory_if(LIBPNG ${AE_EXTERNAL_DIR_NAME}/libpng)
__add_subdirectory_if(MIMALLOC ${AE_EXTERNAL_DIR_NAME}/mimalloc)
__add_subdirectory_if(SPIRV_CROSS ${AE_EXTERNAL_DIR_NAME}/SPIRV-Cross)
__add_subdirectory_if(TCMALLOC ${AE_EXTERNAL_DIR_NAME}/tcmalloc)

__add_subdirectory_if(AE_CORE Core)
__add_subdirectory_if(AE_FRAMEWORK Framework)

__add_subdirectory_if(AE_SHADER_TRANSPILER ${AE_MODULES_DIR_NAME}/Graphics/ShaderTranspiler)#??only x64

__add_subdirectory_if(AE_GRAPHICS_D3D11 ${AE_MODULES_DIR_NAME}/Graphics/Direct3D11)
__add_subdirectory_if(AE_GRAPHICS_GL ${AE_MODULES_DIR_NAME}/Graphics/OpenGL)
__add_subdirectory_if(AE_GRAPHICS_VULKAN ${AE_MODULES_DIR_NAME}/Graphics/Vulkan)

__add_subdirectory_if(AE_HID ${AE_EXTENSIONS_DIR_NAME}/HID)

__add_subdirectory_if(AE_INPUT_DIRECT_INPUT ${AE_MODULES_DIR_NAME}/Inputs/DirectInput)
__add_subdirectory_if(AE_INPUT_HID_INPUT ${AE_MODULES_DIR_NAME}/Inputs/HIDInput)
__add_subdirectory_if(AE_INPUT_RAW_INPUT ${AE_MODULES_DIR_NAME}/Inputs/RawInput)
__add_subdirectory_if(AE_INPUT_XINPUT ${AE_MODULES_DIR_NAME}/Inputs/XInput)

__add_subdirectory_if(AE_ASTC_CONVERTER ${AE_EXTENSIONS_DIR_NAME}/ASTCConverter)
__add_subdirectory_if(AE_PNG_CONVERTER ${AE_EXTENSIONS_DIR_NAME}/PNGConverter)
__add_subdirectory_if(AE_FBX_CONVERTER ${AE_EXTENSIONS_DIR_NAME}/FBXConverter)
__add_subdirectory_if(AE_SHADER_SCRIPT ${AE_EXTENSIONS_DIR_NAME}/ShaderScript)

__add_subdirectory_if(AE_TESTS Test)

if (AE_INSTALL)
    __sub_dir_list(${PROJECT_BINARY_DIR}/${AE_EXTERNAL_DIR_NAME} projs)
    
    foreach(proj ${projs})
        install(DIRECTORY ${PROJECT_BINARY_DIR}/${AE_EXTERNAL_DIR_NAME}/${proj}/install/ DESTINATION ${AE_EXTERNAL_DIR_NAME}/${proj})
    endforeach()
endif ()