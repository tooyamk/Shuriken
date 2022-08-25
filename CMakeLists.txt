cmake_minimum_required (VERSION 3.20.0)

set(SRK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
project(Shuriken)
set(SRK_PROJ_PREFIX Srk-)
set(CMAKE_INSTALL_PREFIX ${SRK_INSTALL_PREFIX})
set(CMAKE_SKIP_RPATH True)
set(CMAKE_XCODE_ATTRIBUTE_DYLIB_INSTALL_NAME_BASE @rpath)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0068 NEW)
cmake_policy(SET CMP0114 NEW)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX rd)
set(CMAKE_MINSIZEREL_POSTFIX rs)
set(SRK_BUILD_TYPE_SUFFIX $<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>$<$<CONFIG:Release>:${CMAKE_RELEASE_POSTFIX}>$<$<CONFIG:RelWithDebInfo>:${CMAKE_RELWITHDEBINFO_POSTFIX}>$<$<CONFIG:MinSizeRel>:${CMAKE_MINSIZEREL_POSTFIX}>)
set(SRK_BUILD_TYPE_DIR $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>$<$<CONFIG:MinSizeRel>:MinSizeRel>)
set(SRK_INSTALL_DLL_DIR_NAME $<IF:$<PLATFORM_ID:Windows>,bin,lib>)

set(SRK_OS_WINDOWS False)
set(SRK_OS_LINUX False)
set(SRK_OS_MACOS False)
set(SRK_OS_ANDROID False)
set(SRK_OS_IOS False)
set(SRK_OS_UNKNOWN False)

set(SRK_HOST_OS_WINDOWS False)
set(SRK_HOST_OS_LINUX False)
set(SRK_HOST_OS_MACOS False)
set(SRK_HOST_OS_UNKNOWN False)

function(srkDetectOS value prefix)
    string(TOLOWER ${value} ${value})
    if (${value} MATCHES "windows")
        set(${prefix}WINDOWS True PARENT_SCOPE)
    elseif (${value} MATCHES "linux")
        set(${prefix}LINUX True PARENT_SCOPE)
    elseif (${value} MATCHES "darwin")
        set(${prefix}MACOS True PARENT_SCOPE)
    elseif (${value} MATCHES "android")
        set(${prefix}ANDROID True PARENT_SCOPE)
    elseif (${value} MATCHES "ios")
        set(${prefix}IOS True PARENT_SCOPE)
    else ()
        set(${prefix}UNKNOWN True PARENT_SCOPE)
    endif ()
endfunction()

srkDetectOS(${CMAKE_SYSTEM_NAME} SRK_OS_)
srkDetectOS(${CMAKE_HOST_SYSTEM_NAME} SRK_HOST_OS_)

set(SRK_C_COMPILER_CLANG False)
set(SRK_C_COMPILER_GCC False)
set(SRK_C_COMPILER_MSVC False)
set(SRK_C_COMPILER_UNKNOWN False)

set(SRK_CXX_COMPILER_CLANG False)
set(SRK_CXX_COMPILER_GCC False)
set(SRK_CXX_COMPILER_MSVC False)
set(SRK_CXX_COMPILER_UNKNOWN False)

function(srkDetectCompiler value prefix)
    string(TOLOWER ${value} ${value})
    if (${value} MATCHES "clang")
        set(${prefix}CLANG True PARENT_SCOPE)
    elseif (${value} MATCHES "gnu")
        set(${prefix}GCC True PARENT_SCOPE)
    elseif (${value} MATCHES "msvc")
        set(${prefix}MSVC True PARENT_SCOPE)
    else ()
        set(${prefix}UNKNOWN True PARENT_SCOPE)
    endif ()
endfunction()

srkDetectCompiler(${CMAKE_C_COMPILER_ID} SRK_C_COMPILER_)
srkDetectCompiler(${CMAKE_CXX_COMPILER_ID} SRK_CXX_COMPILER_)

set(SRK_ARCH_X86 False)
set(SRK_ARCH_ARM False)
set(SRK_ARCH_UNKNOWN False)

set(SRK_HOST_ARCH_X86 False)
set(SRK_HOST_ARCH_ARM False)
set(SRK_HOST_ARCH_UNKNOWN False)

function(srkDetectArch value prefix)
    string(TOLOWER ${value} ${value})
    if (${value} MATCHES "(amd)|(86)")
        set(${prefix}X86 True PARENT_SCOPE)
    elseif (${value} MATCHES "(arm)|(aarch)")
        set(${prefix}ARM True PARENT_SCOPE)
    else ()
        set(${prefix}UNKNOWN True PARENT_SCOPE)
    endif ()
endfunction()

srkDetectArch(${CMAKE_SYSTEM_PROCESSOR} SRK_ARCH_)
srkDetectArch(${CMAKE_HOST_SYSTEM_PROCESSOR} SRK_HOST_ARCH_)

set(SRK_ARCH_WORD_BITS_32 False)
set(SRK_ARCH_WORD_BITS_64 False)
set(SRK_ARCH_WORD_BITS_UNKNOWN False)

set(SRK_HOST_ARCH_WORD_BITS_32 False)
set(SRK_HOST_ARCH_WORD_BITS_64 False)
set(SRK_HOST_ARCH_WORD_BITS_UNKNOWN False)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(SRK_ARCH_WORD_BITS_64 True)
elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(SRK_ARCH_WORD_BITS_32 True)
else ()
    set(SRK_ARCH_WORD_BITS_UNKNOWN True)
endif ()

if (CMAKE_CL_64)
    set(SRK_HOST_ARCH_WORD_BITS_64 True)
else ()
    set(SRK_HOST_ARCH_WORD_BITS_32 True)
endif ()

find_package(UDev)
find_package(X11)

if (SRK_OS_MACOS)
    find_library(FRAMEWORK_OpenGL NAMES OpenGL PATHS ${CMAKE_OSX_SYSROOT}/System/Library)
    if (${FRAMEWORK_OpenGL} STREQUAL FRAMEWORK_OpenGL-NOTFOUND)
        set(OPENGL_FOUND False)
    else ()
        set(OPENGL_FOUND True)
        set(OPENGL_LIBRARIES ${FRAMEWORK_OpenGL})
    endif ()
else ()
    find_package(OpenGL)
endif ()

macro(srkSetSubProject prefix)
    get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "-" PROJECT_NAME ${PROJECT_NAME})
    set(PROJECT_NAME ${prefix}${PROJECT_NAME})
    #set(PROJECT_NAME ${prefix}${PROJECT_NAME})
    #project(${_projId})

    #set(PROJECT_NAME ${_projId})
    set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    set(SRK_PROJECT_BUILD_ROOT_DIR ${PROJECT_BINARY_DIR}/build)
    set(SRK_PROJECT_BUILD_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}/${SRK_BUILD_TYPE_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SRK_PROJECT_BUILD_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SRK_PROJECT_BUILD_DIR})
    #set(LIBRARY_OUTPUT_PATH ${SRK_PROJECT_BUILD_DIR}$<0:>)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SRK_PROJECT_BUILD_DIR})
    
    #unset(_projId)
endmacro()

function(srkAssignSourceGroup)
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
 
function(srkAddLibrarySourceGroup)
    foreach (_source IN ITEMS ${ARGN})
    srkAssignSourceGroup(${_source})
    endforeach ()
    add_library(${ARGV})
endfunction()

function(srkAddExecutableSourceGroup)
    foreach (_source IN ITEMS ${ARGN})
    srkAssignSourceGroup(${_source})
    endforeach ()
    add_executable(${ARGV})
endfunction()

function(srkTargetLinkLibraryExternalProject target lib_prefix)
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

function(srkGetSubdirectoryList cur dest)
    file(GLOB children RELATIVE ${cur} ${cur}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${cur}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${dest} ${dirlist} PARENT_SCOPE)
endfunction()

function(srkFindParentDir in out names)
    set(cond TRUE)
    while(${cond})
        get_filename_component(name ${in} NAME)
        string(LENGTH "${name}" len)
        if (${len} EQUAL 0)
            break()
        endif ()
        
        foreach(target names)
            if (${target} STREQUAL ${name})
                set(cond FALSE)
                break()
            endif ()
        endforeach()
        
        if (${cond})
            get_filename_component(in ${in} DIRECTORY)
        else ()
            set(${out} ${in} PARENT_SCOPE)
        endif ()
    endwhile()
endfunction()

function(srkCollectSrcFiles rst)
    if (SRK_OS_MACOS)
        file(GLOB_RECURSE _files "src/*.h" "src/*.cpp" "src/*.mm")
    else ()
        file(GLOB_RECURSE _files "src/*.h" "src/*.cpp")
        
        #[[file(GLOB_RECURSE tmp "src/*.mm")
        list(LENGTH tmp count)
        if (${count} GREATER 0)
            foreach(item ${tmp})
                string(REGEX REPLACE ".+/(.+)\\..*" "\\1" _fileName ${item})
                list(FILTER tmp1 EXCLUDE REGEX ${_fileName}.h)
            endforeach()
        endif ()]]
    endif ()
    
    set(${rst} ${_files} PARENT_SCOPE)
endfunction()

function(srkDelayLoadImport dll)
    if (SRK_CXX_COMPILER_MSVC)
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DELAYLOAD:${dll}${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DELAYLOAD:${dll}${CMAKE_RELEASE_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DELAYLOAD:${dll}${CMAKE_RELWITHDEBINFO_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /DELAYLOAD:${dll}${CMAKE_MINSIZEREL_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
    endif ()
endfunction()

function(srkTryDownloadFile url dst hash)
    if (EXISTS ${dst})
        file(SHA256 ${dst} h)
        if (NOT (${h} STREQUAL ${hash}))
            file(DOWNLOAD ${url} ${dst} SHOW_PROGRESS)
        endif ()
    else ()
        file(DOWNLOAD ${url} ${dst} SHOW_PROGRESS)
    endif ()
endfunction()

function(srkSetTargetRpath target rpath)
    if (SRK_OS_MACOS OR SRK_OS_IOS)
        string(REPLACE ":" ";" rpath "${rpath}")
        string(REGEX REPLACE "^[    ]*\./" "@executable_path/" rpath "${rpath}")
        string(REGEX REPLACE ";[    ]*\./" ";@executable_path/" rpath "${rpath}")
    else (SRK_OS_LINUX)
        string(REPLACE ";" ":" rpath "${rpath}")
    endif ()
    
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS ${rpath})
    if ((NOT ${CMAKE_GENERATOR} STREQUAL Xcode) AND (SRK_C_COMPILER_GCC OR SRK_C_COMPILER_CLANG OR SRK_CXX_COMPILER_GCC OR SRK_CXX_COMPILER_CLANG))
        target_link_options(${target} PRIVATE "-Wl,-rpath=${rpath}")
    endif ()
endfunction()

function(srkInstall name)
    if (SRK_ENABLE_INSTALL)
        install(DIRECTORY "src/" DESTINATION ${name}/include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*impl*.*" EXCLUDE PATTERN "private" EXCLUDE)
        install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION ${name}/bin
            ARCHIVE DESTINATION ${name}/lib
            LIBRARY DESTINATION ${name}/lib
        )
    endif ()
endfunction()

function(srkAppendIf cond arr value)
    if (${cond})
        list(APPEND ${arr} ${value})
        set(${arr} ${${arr}} PARENT_SCOPE)
    endif ()
endfunction()

function(srkAppendCopyFileCmd cmd src dst)
    list(APPEND ${cmd} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst})
    set(${cmd} ${${cmd}} PARENT_SCOPE)
endfunction()

function(srkAppendCopyFileCmdIf cond cmd src dst)
    if (${cond})
        list(APPEND ${cmd} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst})
        set(${cmd} ${${cmd}} PARENT_SCOPE)
    endif ()
endfunction()

function(srkAddSubdirectoryIf cond dir)
    if (${cond})
        add_subdirectory(${dir})
    endif ()
endfunction()

option(SRK_ENABLE_EXTERNAL_7Z "" OFF)
option(SRK_ENABLE_EXTERNAL_ASTC_ENCODER "" OFF)
option(SRK_ENABLE_EXTERNAL_DX_SHADER_COMPILER "" OFF)
option(SRK_ENABLE_EXTERNAL_GLEW "" OFF)
option(SRK_ENABLE_EXTERNAL_LIBPNG "" OFF)
option(SRK_ENABLE_EXTERNAL_MIMALLOC "" OFF)
option(SRK_ENABLE_EXTERNAL_SPIRV_CROSS "" OFF)
option(SRK_ENABLE_EXTERNAL_TCMALLOC "" OFF)
option(SRK_ENABLE_EXTERNAL_VULKAN_SDK "" OFF)
option(SRK_ENABLE_EXTERNAL_ZLIB "" OFF)

option(SRK_ENABLE_CORE "" ON)
option(SRK_ENABLE_FRAMEWORK "" ON)

option(SRK_ENABLE_ALL_MODULES "" OFF)

option(SRK_ENABLE_MODULE_SHADER_TRANSPILER "" OFF)
option(SRK_ENABLE_MODULE_GRAPHICS_D3D11 "" ON)
option(SRK_ENABLE_MODULE_GRAPHICS_GL "" ON)
option(SRK_ENABLE_MODULE_GRAPHICS_VULKAN "" ON)

option(SRK_ENABLE_MODULE_INPUT_DIRECT_INPUT "" OFF)
option(SRK_ENABLE_MODULE_INPUTS_HID_INPUT "" OFF)
option(SRK_ENABLE_MODULE_INPUTS_RAW_INPUT "" OFF)
option(SRK_ENABLE_MODULE_INPUTS_XINPUT "" OFF)

option(SRK_ENABLE_MODULE_WINDOWS_COCOA "" OFF)
option(SRK_ENABLE_MODULE_WINDOWS_WIN32API "" OFF)
option(SRK_ENABLE_MODULE_WINDOWS_X11 "" OFF)

option(SRK_ENABLE_ALL_EXTENSIONS "" OFF)

option(SRK_ENABLE_EXTENSION_ASTC_CONVERTER "" OFF)
option(SRK_ENABLE_EXTENSION_FBX_CONVERTER "" OFF)
option(SRK_ENABLE_EXTENSION_HID "" OFF)
option(SRK_ENABLE_EXTENSION_PNG_CONVERTER "" OFF)
option(SRK_ENABLE_EXTENSION_SHADER_SCRIPT "" OFF)

option(SRK_ENABLE_TESTS "" OFF)

option(SRK_ENABLE_INSTALL "" ON)

if (SRK_ENABLE_TESTS)
    set(SRK_ENABLE_ALL_MODULES ON)
    set(SRK_ENABLE_ALL_EXTENSIONS ON)
    set(SRK_ENABLE_EXTERNAL_MIMALLOC ON)
    set(SRK_ENABLE_EXTERNAL_TCMALLOC ON)
endif ()

if (SRK_ENABLE_ALL_MODULES)
    set(SRK_ENABLE_MODULE_SHADER_TRANSPILER ON)
    set(SRK_ENABLE_MODULE_GRAPHICS_D3D11 ON)
    set(SRK_ENABLE_MODULE_GRAPHICS_GL ON)
    set(SRK_ENABLE_MODULE_GRAPHICS_VULKAN ON)

    set(SRK_ENABLE_MODULE_INPUT_DIRECT_INPUT ON)
    set(SRK_ENABLE_MODULE_INPUTS_HID_INPUT ON)
    set(SRK_ENABLE_MODULE_INPUTS_RAW_INPUT ON)
    set(SRK_ENABLE_MODULE_INPUTS_XINPUT ON)

    set(SRK_ENABLE_MODULE_WINDOWS_COCOA ON)
    set(SRK_ENABLE_MODULE_WINDOWS_WIN32API ON)
    set(SRK_ENABLE_MODULE_WINDOWS_X11 ON)
endif ()

if (SRK_ENABLE_ALL_EXTENSIONS)
    set(SRK_ENABLE_EXTENSION_ASTC_CONVERTER ON)
    set(SRK_ENABLE_EXTENSION_FBX_CONVERTER ON)
    set(SRK_ENABLE_EXTENSION_HID ON)
    set(SRK_ENABLE_EXTENSION_PNG_CONVERTER ON)
    set(SRK_ENABLE_EXTENSION_SHADER_SCRIPT ON)
endif ()

#validity check
#==========
if (NOT SRK_OS_WINDOWS)
    set(SRK_ENABLE_MODULE_GRAPHICS_D3D11 OFF)

    set(SRK_ENABLE_MODULE_INPUT_DIRECT_INPUT OFF)
    set(SRK_ENABLE_MODULE_INPUTS_RAW_INPUT OFF)
    set(SRK_ENABLE_MODULE_INPUTS_XINPUT OFF)

    set(SRK_ENABLE_MODULE_WINDOWS_WIN32API OFF)
endif ()

if (SRK_OS_LINUX)
    if (NOT X11_FOUND)
        set(SRK_ENABLE_MODULE_GRAPHICS_VULKAN OFF)
        set(SRK_ENABLE_MODULE_WINDOWS_X11 OFF)
    endif ()
else ()
    set(SRK_ENABLE_MODULE_WINDOWS_X11 OFF)
endif ()

if (NOT SRK_OS_MACOS)
    set(SRK_ENABLE_MODULE_WINDOWS_COCOA OFF)
endif ()

if (UNIX)
    if (SRK_ENABLE_EXTENSION_HID OR SRK_ENABLE_MODULE_INPUTS_HID_INPUT)
        if (NOT UDEV_FOUND)
            set(SRK_ENABLE_EXTENSION_HID OFF)
            set(SRK_ENABLE_MODULE_INPUTS_HID_INPUT OFF)
        endif ()
    endif ()
endif ()

if (SRK_ENABLE_EXTERNAL_GLEW OR SRK_ENABLE_MODULE_GRAPHICS_GL)
    if (NOT OPENGL_FOUND)
        set(SRK_ENABLE_EXTERNAL_GLEW OFF)
        set(SRK_ENABLE_MODULE_GRAPHICS_GL OFF)
    endif ()
endif ()
#==========

#dependence check
#==========
if (SRK_ENABLE_EXTENSION_SHADER_SCRIPT)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_EXTENSION_ASTC_CONVERTER)
    set(SRK_ENABLE_EXTERNAL_ASTC_ENCODER ON)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_EXTENSION_FBX_CONVERTER)
    set(SRK_ENABLE_EXTERNAL_ZLIB ON)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_EXTENSION_PNG_CONVERTER)
    set(SRK_ENABLE_EXTERNAL_LIBPNG ON)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_GRAPHICS_D3D11)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_GRAPHICS_GL)
    set(SRK_ENABLE_EXTERNAL_GLEW ON)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_GRAPHICS_VULKAN)
    set(SRK_ENABLE_FRAMEWORK ON)
    set(SRK_ENABLE_EXTERNAL_VULKAN_SDK ON)
endif ()

if (SRK_ENABLE_MODULE_SHADER_TRANSPILER)
    set(SRK_ENABLE_EXTERNAL_DX_SHADER_COMPILER ON)
    set(SRK_ENABLE_EXTERNAL_SPIRV_CROSS ON)
endif ()

if (SRK_ENABLE_MODULE_INPUTS_HID_INPUT)
    set(SRK_ENABLE_FRAMEWORK ON)
    set(SRK_ENABLE_EXTENSION_HID ON)
endif ()

if (SRK_ENABLE_MODULE_INPUT_DIRECT_INPUT)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_INPUTS_RAW_INPUT)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_INPUTS_XINPUT)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_WINDOWS_COCOA)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_WINDOWS_WIN32API)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_MODULE_WINDOWS_X11)
    set(SRK_ENABLE_FRAMEWORK ON)
endif ()

if (SRK_ENABLE_FRAMEWORK)
    set(SRK_ENABLE_CORE ON)
endif ()

if (SRK_ENABLE_EXTERNAL_LIBPNG)
    set(SRK_ENABLE_EXTERNAL_ZLIB ON)
endif ()

if (SRK_ENABLE_EXTERNAL_VULKAN_SDK AND (SRK_OS_WINDOWS OR SRK_OS_MACOS))
    set(SRK_ENABLE_EXTERNAL_7Z ON)
endif ()
#==========

#set(CMAKE_SHARED_LIBRARY_PREFIX "")
#set(CMAKE_STATIC_LIBRARY_PREFIX "")
#set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "")
#set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "")
#set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "")
#set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_ATTRIBUTE_GCC_C_LANGUAGE_STANDARD c11)
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO)

set(compilerFlags "-fms-extensions -fPIC -Wno-unused-command-line-argument -Wno-switch")
if (SRK_C_COMPILER_CLANG OR SRK_C_COMPILER_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compilerFlags}")
endif ()
if (SRK_CXX_COMPILER_CLANG OR SRK_CXX_COMPILER_GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compilerFlags}")
endif ()

set(compilerFlags " /wd4244 /wd4251 /wd4267 /wd4275 /wd4819 /wd4838 /wd4996 /wd26812")
if (SRK_C_COMPILER_MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compilerFlags}")
endif ()
if (SRK_CXX_COMPILER_MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compilerFlags}")
endif ()

unset(compilerFlags)

set(SRK_DEFINITIONS -DSRK_$<$<CONFIG:Debug>:DEBUG>$<$<CONFIG:Release>:RELEASE>$<$<CONFIG:RELEASE_WITH_DEBUG_INFO>:RelWithDebInfo>$<$<CONFIG:MinSizeRel>:MIN_SIZE_RELEASE>)
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    list(APPEND SRK_DEFINITIONS -DSRK_DEBUG)
endif ()

set(SRK_MODULE_DEFINITIONS ${SRK_DEFINITIONS} -DSRK_MODULE_EXPORTS)
set(SRK_EXTENSION_DEFINITIONS ${SRK_DEFINITIONS} -DSRK_EXTENSION_EXPORTS)

if (SRK_OS_WINDOWS)
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
set(SRK_MODULES_GRAPHICS_DIR ${SRK_MODULES_DIR_NAME}/Graphics)
set(SRK_MODULES_INPUTS_DIR ${SRK_MODULES_DIR_NAME}/Inputs)
set(SRK_MODULES_WINDOWS_DIR ${SRK_MODULES_DIR_NAME}/Windows)
set(SRK_EXTENSIONS_DIR_NAME Extensions)

set(SRK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

macro(srkCreateExternalProjectCmakeArgs target)
    set(${target}
        -DCMAKE_POLICY_DEFAULT_CMP0042=NEW
        -DCMAKE_POLICY_DEFAULT_CMP0068=NEW
        -DCMAKE_POLICY_DEFAULT_CMP0114=NEW
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
        -DCMAKE_RELEASE_POSTFIX=${CMAKE_RELEASE_POSTFIX}
        -DCMAKE_RELWITHDEBINFO_POSTFIX=${CMAKE_RELWITHDEBINFO_POSTFIX}
        -DCMAKE_MINSIZEREL_POSTFIX=${CMAKE_MINSIZEREL_POSTFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        #-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_SKIP_RPATH=${CMAKE_SKIP_RPATH}
        -DCMAKE_XCODE_ATTRIBUTE_DYLIB_INSTALL_NAME_BASE=${CMAKE_XCODE_ATTRIBUTE_DYLIB_INSTALL_NAME_BASE}
        -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DANDROID_ABI=${ANDROID_ABI}
        -DANDROID_PLATFORM=${ANDROID_PLATFORM}
        -DANDROID_NDK=${ANDROID_NDK}
    )
endmacro()

srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_7Z ${SRK_EXTERNAL_DIR_NAME}/7z)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_ASTC_ENCODER ${SRK_EXTERNAL_DIR_NAME}/astc-encoder)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_DX_SHADER_COMPILER ${SRK_EXTERNAL_DIR_NAME}/DirectXShaderCompiler)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_GLEW ${SRK_EXTERNAL_DIR_NAME}/glew)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_ZLIB ${SRK_EXTERNAL_DIR_NAME}/zlib)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_LIBPNG ${SRK_EXTERNAL_DIR_NAME}/libpng)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_MIMALLOC ${SRK_EXTERNAL_DIR_NAME}/mimalloc)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_SPIRV_CROSS ${SRK_EXTERNAL_DIR_NAME}/SPIRV-Cross)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_TCMALLOC ${SRK_EXTERNAL_DIR_NAME}/tcmalloc)
srkAddSubdirectoryIf(SRK_ENABLE_EXTERNAL_VULKAN_SDK ${SRK_EXTERNAL_DIR_NAME}/VulkanSDK)

srkAddSubdirectoryIf(SRK_ENABLE_CORE Core)
srkAddSubdirectoryIf(SRK_ENABLE_FRAMEWORK Framework)

srkAddSubdirectoryIf(SRK_ENABLE_MODULE_SHADER_TRANSPILER ${SRK_MODULES_GRAPHICS_DIR}/ShaderTranspiler)#??only x64

srkAddSubdirectoryIf(SRK_ENABLE_MODULE_GRAPHICS_D3D11 ${SRK_MODULES_GRAPHICS_DIR}/Direct3D11)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_GRAPHICS_GL ${SRK_MODULES_GRAPHICS_DIR}/OpenGL)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_GRAPHICS_VULKAN ${SRK_MODULES_GRAPHICS_DIR}/Vulkan)

srkAddSubdirectoryIf(SRK_ENABLE_EXTENSION_HID ${SRK_EXTENSIONS_DIR_NAME}/HID)

srkAddSubdirectoryIf(SRK_ENABLE_MODULE_INPUT_DIRECT_INPUT ${SRK_MODULES_INPUTS_DIR}/DirectInput)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_INPUTS_HID_INPUT ${SRK_MODULES_INPUTS_DIR}/HIDInput)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_INPUTS_RAW_INPUT ${SRK_MODULES_INPUTS_DIR}/RawInput)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_INPUTS_XINPUT ${SRK_MODULES_INPUTS_DIR}/XInput)

srkAddSubdirectoryIf(SRK_ENABLE_MODULE_WINDOWS_COCOA ${SRK_MODULES_WINDOWS_DIR}/Cocoa)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_WINDOWS_WIN32API ${SRK_MODULES_WINDOWS_DIR}/Win32API)
srkAddSubdirectoryIf(SRK_ENABLE_MODULE_WINDOWS_X11 ${SRK_MODULES_WINDOWS_DIR}/X11)

srkAddSubdirectoryIf(SRK_ENABLE_EXTENSION_ASTC_CONVERTER ${SRK_EXTENSIONS_DIR_NAME}/ASTCConverter)
srkAddSubdirectoryIf(SRK_ENABLE_EXTENSION_PNG_CONVERTER ${SRK_EXTENSIONS_DIR_NAME}/PNGConverter)
srkAddSubdirectoryIf(SRK_ENABLE_EXTENSION_FBX_CONVERTER ${SRK_EXTENSIONS_DIR_NAME}/FBXConverter)
srkAddSubdirectoryIf(SRK_ENABLE_EXTENSION_SHADER_SCRIPT ${SRK_EXTENSIONS_DIR_NAME}/ShaderScript)

srkAddSubdirectoryIf(SRK_ENABLE_TESTS Test)

if (SRK_ENABLE_INSTALL)
    srkGetSubdirectoryList(${PROJECT_BINARY_DIR}/${SRK_EXTERNAL_DIR_NAME} projs)
    
    foreach(proj ${projs})
        install(DIRECTORY ${PROJECT_BINARY_DIR}/${SRK_EXTERNAL_DIR_NAME}/${proj}/install/ DESTINATION ${SRK_EXTERNAL_DIR_NAME}/${proj})
    endforeach()
endif ()