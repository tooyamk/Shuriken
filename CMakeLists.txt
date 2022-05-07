cmake_minimum_required (VERSION 3.15.0)

set(SRK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
project(Shuriken)
set(CMAKE_INSTALL_PREFIX ${SRK_INSTALL_PREFIX})
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

    set(SRK_BUILD_DIR ${PROJECT_BINARY_DIR}/build/)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SRK_BUILD_DIR}$<0:>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SRK_BUILD_DIR}$<0:>)
    #set(LIBRARY_OUTPUT_PATH ${SRK_BUILD_DIR}$<0:>)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SRK_BUILD_DIR}$<0:>)
    
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
    if (SRK_INSTALL)
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
        list(APPEND ${arr} ${value})
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

option(SRK_CORE "" ON)
option(SRK_FRAMEWORK "" ON)

option(SRK_ALL_MODULES "" OFF)

option(SRK_SHADER_TRANSPILER "" OFF)
option(SRK_GRAPHICS_D3D11 "" ON)
option(SRK_GRAPHICS_GL "" ON)
option(SRK_GRAPHICS_VULKAN "" ON)

option(SRK_INPUT_HID_INPUT "" OFF)
option(SRK_INPUT_DIRECT_INPUT "" OFF)
option(SRK_INPUT_RAW_INPUT "" OFF)
option(SRK_INPUT_XINPUT "" OFF)

option(SRK_ALL_EXTENSIONS "" OFF)

option(SRK_ASTC_CONVERTER "" OFF)
option(SRK_FBX_CONVERTER "" OFF)
option(SRK_HID "" OFF)
option(SRK_PNG_CONVERTER "" OFF)
option(SRK_SHADER_SCRIPT "" OFF)

option(SRK_TESTS "" OFF)

option(SRK_INSTALL "" ON)

if (SRK_TESTS)
    set(SRK_ALL_MODULES ON)
    set(SRK_ALL_EXTENSIONS ON)
    set(MIMALLOC ON)
    set(TCMALLOC ON)
endif ()

if (SRK_ALL_MODULES)
    set(SRK_SHADER_TRANSPILER ON)
    set(SRK_GRAPHICS_D3D11 ON)
    set(SRK_GRAPHICS_GL ON)
    set(SRK_GRAPHICS_VULKAN ON)

    set(SRK_INPUT_DIRECT_INPUT ON)
    set(SRK_INPUT_HID_INPUT ON)
    set(SRK_INPUT_RAW_INPUT ON)
    set(SRK_INPUT_XINPUT ON)
endif ()

if (SRK_ALL_EXTENSIONS)
    set(SRK_ASTC_CONVERTER ON)
    set(SRK_FBX_CONVERTER ON)
    set(SRK_HID ON)
    set(SRK_PNG_CONVERTER ON)
    set(SRK_SHADER_SCRIPT ON)
endif ()

#validity check
#==========
if (NOT WIN32)
    set(SRK_GRAPHICS_D3D11 OFF)

    set(SRK_INPUT_DIRECT_INPUT OFF)
    set(SRK_INPUT_RAW_INPUT OFF)
    set(SRK_INPUT_XINPUT OFF)
endif ()

if (UNIX)
    if (SRK_HID OR SRK_INPUT_HID_INPUT)
        if (NOT UDEV_FOUND)
            set(SRK_HID OFF)
            set(SRK_INPUT_HID_INPUT OFF)
        endif ()
    endif ()
endif ()

if (GLEW OR SRK_GRAPHICS_GL)
    if (NOT OPENGL_FOUND)
        set(GLEW OFF)
        set(SRK_GRAPHICS_GL OFF)
    endif ()
endif ()

if (SRK_GRAPHICS_VULKAN)
    if (NOT Vulkan_FOUND)
        set(SRK_GRAPHICS_VULKAN OFF)
    endif ()
endif ()
#==========

#dependence check
#==========
if (SRK_SHADER_SCRIPT)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_ASTC_CONVERTER)
    set(ASTC_ENCODER ON)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_FBX_CONVERTER)
    set(ZLIB ON)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_INPUT_HID_INPUT)
    set(SRK_FRAMEWORK ON)
    set(SRK_HID ON)
endif ()

if (SRK_PNG_CONVERTER)
    set(LIBPNG ON)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_INPUT_DIRECT_INPUT)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_INPUT_RAW_INPUT)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_INPUT_XINPUT)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_GRAPHICS_D3D11)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_GRAPHICS_GL)
    set(GLEW ON)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_GRAPHICS_VULKAN)
    set(SRK_FRAMEWORK ON)
endif ()

if (SRK_SHADER_TRANSPILER)
    set(DX_SHADER_COMPILER ON)
    set(SPIRV_CROSS ON)
endif ()

if (SRK_FRAMEWORK)
    set(SRK_CORE ON)
endif ()

if (LIBPNG)
    set(ZLIB ON)
endif ()
#==========

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "")#rd)
set(CMAKE_MINSIZEREL_POSTFIX "")#rs)
set(SRK_BUILD_TYPE_SUFFIX $<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>$<$<CONFIG:Release>:${CMAKE_RELEASE_POSTFIX}>$<$<CONFIG:RelWithDebInfo>:${CMAKE_RELWITHDEBINFO_POSTFIX}>$<$<CONFIG:MinSizeRel>:${CMAKE_MINSIZEREL_POSTFIX}>)
set(SRK_INSTALL_DLL_DIR_NAME $<IF:$<PLATFORM_ID:Windows>,bin,lib>)

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

set(SRK_DEFINITIONS)
if (CMAKE_BUILD_TYPE MATCHES "Deb")
    LIST(APPEND SRK_DEFINITIONS -DSRK_DEBUG)
endif ()

set(SRK_MODULE_DEFINITIONS ${SRK_DEFINITIONS} -DSRK_MODULE_EXPORTS)
set(SRK_EXTENSION_DEFINITIONS ${SRK_DEFINITIONS} -DSRK_EXTENSION_EXPORTS)

if (WIN32)
    set(SRK_SLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(SRK_LIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(SRK_DLL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else ()
    set(SRK_SLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(SRK_LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(SRK_DLL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif ()

set(SRK_EXTERNAL_DIR_NAME External)
set(SRK_MODULES_DIR_NAME Modules)
set(SRK_GRAPHICS_MODULES_DIR ${SRK_MODULES_DIR_NAME}/Graphics)
set(SRK_INPUT_MODULES_DIR ${SRK_MODULES_DIR_NAME}/Inputs)
set(SRK_EXTENSIONS_DIR_NAME Extensions)

set(SRK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

__add_subdirectory_if(ASTC_ENCODER ${SRK_EXTERNAL_DIR_NAME}/astc-encoder)
__add_subdirectory_if(DX_SHADER_COMPILER ${SRK_EXTERNAL_DIR_NAME}/DirectXShaderCompiler)
__add_subdirectory_if(GLEW ${SRK_EXTERNAL_DIR_NAME}/glew)
__add_subdirectory_if(ZLIB ${SRK_EXTERNAL_DIR_NAME}/zlib)
__add_subdirectory_if(LIBPNG ${SRK_EXTERNAL_DIR_NAME}/libpng)
__add_subdirectory_if(MIMALLOC ${SRK_EXTERNAL_DIR_NAME}/mimalloc)
__add_subdirectory_if(SPIRV_CROSS ${SRK_EXTERNAL_DIR_NAME}/SPIRV-Cross)
__add_subdirectory_if(TCMALLOC ${SRK_EXTERNAL_DIR_NAME}/tcmalloc)

__add_subdirectory_if(SRK_CORE Core)
__add_subdirectory_if(SRK_FRAMEWORK Framework)

__add_subdirectory_if(SRK_SHADER_TRANSPILER ${SRK_MODULES_DIR_NAME}/Graphics/ShaderTranspiler)#??only x64

__add_subdirectory_if(SRK_GRAPHICS_D3D11 ${SRK_MODULES_DIR_NAME}/Graphics/Direct3D11)
__add_subdirectory_if(SRK_GRAPHICS_GL ${SRK_MODULES_DIR_NAME}/Graphics/OpenGL)
__add_subdirectory_if(SRK_GRAPHICS_VULKAN ${SRK_MODULES_DIR_NAME}/Graphics/Vulkan)

__add_subdirectory_if(SRK_HID ${SRK_EXTENSIONS_DIR_NAME}/HID)

__add_subdirectory_if(SRK_INPUT_DIRECT_INPUT ${SRK_MODULES_DIR_NAME}/Inputs/DirectInput)
__add_subdirectory_if(SRK_INPUT_HID_INPUT ${SRK_MODULES_DIR_NAME}/Inputs/HIDInput)
__add_subdirectory_if(SRK_INPUT_RAW_INPUT ${SRK_MODULES_DIR_NAME}/Inputs/RawInput)
__add_subdirectory_if(SRK_INPUT_XINPUT ${SRK_MODULES_DIR_NAME}/Inputs/XInput)

__add_subdirectory_if(SRK_ASTC_CONVERTER ${SRK_EXTENSIONS_DIR_NAME}/ASTCConverter)
__add_subdirectory_if(SRK_PNG_CONVERTER ${SRK_EXTENSIONS_DIR_NAME}/PNGConverter)
__add_subdirectory_if(SRK_FBX_CONVERTER ${SRK_EXTENSIONS_DIR_NAME}/FBXConverter)
__add_subdirectory_if(SRK_SHADER_SCRIPT ${SRK_EXTENSIONS_DIR_NAME}/ShaderScript)

__add_subdirectory_if(SRK_TESTS Test)

if (SRK_INSTALL)
    __sub_dir_list(${PROJECT_BINARY_DIR}/${SRK_EXTERNAL_DIR_NAME} projs)
    
    foreach(proj ${projs})
        install(DIRECTORY ${PROJECT_BINARY_DIR}/${SRK_EXTERNAL_DIR_NAME}/${proj}/install/ DESTINATION ${SRK_EXTERNAL_DIR_NAME}/${proj})
    endforeach()
endif ()