cmake_minimum_required (VERSION 3.10.0)

set(AE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
project(Aurora)
set(CMAKE_INSTALL_PREFIX ${AE_INSTALL_PREFIX})

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

option(ZLIB "" ON)
option(LIBPNG "" ON)
option(GLEW "" ON)
option(DX_SHADER_COMPILER "" ON)
option(SPIRV_CROSS "" ON)
option(BOOST_PREPROCESSOR "" ON)

option(AE_CORE "" ON)
option(AE_FRAMEWORK "" ON)

option(AE_PROGRAM_SOURCE_TRANSLATOR "" ON)
if (WIN32)
    option(AE_GRAPHICS_D3D11 "" ON)
    option(AE_GRAPHICS_GL "" ON)
    option(AE_INPUT_DIRECT_INPUT "" ON)
    option(AE_INPUT_XINPUT "" ON)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(AE_GRAPHICS_GL "" ON)
endif ()

option(AE_PNG_CONVERTER "" ON)
option(AE_FBX_CONVERTER "" ON)
option(AE_SHADER_SCRIPT "" ON)

option(AE_TESTS "" OFF)

option(AE_INSTALL "" ON)

if (AE_SHADER_SCRIPT)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_FBX_CONVERTER)
    set(ZLIB ON)
    set(BOOST_PREPROCESSOR ON)
    set(AE_FRAMEWORK ON)
endif ()

if (AE_PNG_CONVERTER)
    set(LIBPNG ON)
    set(AE_FRAMEWORK ON)
endif ()

if (WIN32)
    if (AE_INPUT_XINPUT)
        set(AE_FRAMEWORK ON)
    endif ()

    if (AE_INPUT_DIRECT_INPUT)
        set(AE_FRAMEWORK ON)
    endif ()

    if (AE_GRAPHICS_GL)
        set(GLEW ON)
        set(AE_FRAMEWORK ON)
    endif ()

    if (AE_GRAPHICS_D3D11)
        set(AE_FRAMEWORK ON)
    endif ()
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    if (AE_GRAPHICS_GL)
        set(GLEW ON)
        set(AE_FRAMEWORK ON)
    endif ()
endif ()

if (AE_PROGRAM_SOURCE_TRANSLATOR)
    set(DX_SHADER_COMPILER ON)
    set(SPIRV_CROSS ON)
endif ()

if (AE_FRAMEWORK)
    set(AE_CORE ON)
endif ()

if (LIBPNG)
    set(ZLIB ON)
endif ()

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

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(clangFlags "-fms-extensions -fPIC -Wno-unused-command-line-argument -Wno-switch")

if (CMAKE_C_COMPILER_ID MATCHES Clang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${clangFlags}")
endif ()
if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${clangFlags}")
endif ()

unset(clangFlags)
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

if (ZLIB)
    add_subdirectory(${AE_EXTERNAL_DIR_NAME}/zlib)
endif ()
if (LIBPNG)
    add_subdirectory(${AE_EXTERNAL_DIR_NAME}/libpng)
endif ()
if (GLEW)
    add_subdirectory(${AE_EXTERNAL_DIR_NAME}/glew)
endif ()
if (DX_SHADER_COMPILER)
    add_subdirectory(${AE_EXTERNAL_DIR_NAME}/DirectXShaderCompiler)
endif ()
if (SPIRV_CROSS)
    add_subdirectory(${AE_EXTERNAL_DIR_NAME}/SPIRV-Cross)
endif ()
if (BOOST_PREPROCESSOR)
    add_subdirectory(${AE_EXTERNAL_DIR_NAME}/BoostPreprocessor)
endif ()

if (AE_CORE)
    add_subdirectory(Core)
endif ()
if (AE_FRAMEWORK)
    add_subdirectory(Framework)
endif ()

if (AE_PROGRAM_SOURCE_TRANSLATOR)
    add_subdirectory(${AE_MODULES_DIR_NAME}/Graphics/ProgramSourceTranslator)#??only x64
endif ()
if (WIN32)
    if (AE_GRAPHICS_D3D11)
        add_subdirectory(${AE_MODULES_DIR_NAME}/Graphics/Direct3D11)
    endif ()
    if (AE_GRAPHICS_GL)
        add_subdirectory(${AE_MODULES_DIR_NAME}/Graphics/OpenGL)
    endif ()

    if (AE_INPUT_DIRECT_INPUT)
        add_subdirectory(${AE_MODULES_DIR_NAME}/Inputs/DirectInput)
    endif ()
    if (AE_INPUT_XINPUT)
        add_subdirectory(${AE_MODULES_DIR_NAME}/Inputs/XInput)
    endif ()
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    if (AE_GRAPHICS_GL)
        add_subdirectory(${AE_MODULES_DIR_NAME}/Graphics/OpenGL)
    endif ()
endif ()

if (AE_PNG_CONVERTER)
    add_subdirectory(${AE_EXTENSIONS_DIR_NAME}/PNGConverter)
endif ()
if (AE_FBX_CONVERTER)
    add_subdirectory(${AE_EXTENSIONS_DIR_NAME}/FBXConverter)
endif ()
if (AE_SHADER_SCRIPT)
    add_subdirectory(${AE_EXTENSIONS_DIR_NAME}/ShaderScript)
endif ()

if (AE_TESTS)
    add_subdirectory(Test)
endif ()

if (AE_INSTALL)
    __sub_dir_list(${PROJECT_BINARY_DIR}/${AE_EXTERNAL_DIR_NAME} projs)
    
    foreach(proj ${projs})
        install(DIRECTORY ${PROJECT_BINARY_DIR}/${AE_EXTERNAL_DIR_NAME}/${proj}/install/ DESTINATION ${AE_EXTERNAL_DIR_NAME}/${proj})
    endforeach()
endif ()