__set_project()
set(LLVMDxcSupport LLVMDxcSupport)
if (UNIX)
    set(LLVMDxcSupport lib${LLVMDxcSupport})
endif ()
set(LLVMDxcSupport_SLIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/${LLVMDxcSupport})
set(LLVMDxcSupport_SLIB_R ${LLVMDxcSupport_SLIB_PREFIX}${AE_SLIB_SUFFIX})
set(LLVMDxcSupport_SLIB_D ${LLVMDxcSupport_SLIB_PREFIX}d${AE_SLIB_SUFFIX})
set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_INCLUDE ${PROJECT_BINARY_DIR}/src/include)
set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_PLATFORM_INCLUDE ${PROJECT_BINARY_DIR}/src/include)
set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_LLVMDxcSupport_SLIB_R ${LLVMDxcSupport_SLIB_R})
set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_LLVMDxcSupport_SLIB_D ${LLVMDxcSupport_SLIB_D})
set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_DLL ${PROJECT_BINARY_DIR}/install/bin/dxcompiler${AE_BUILD_TYPE_SUFFIX}${AE_DLL_SUFFIX})

function(__find_dir in out names)
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
endfunction(__find_dir)

set(DIASDK_LIBRARIES "")
set(DIASDK_INCLUDE_DIR "")
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wv:18")

    set(path "nil")
    __find_dir(${CMAKE_CXX_COMPILER} path "Community" "Professional" "Enterprise")
    if (NOT (${path} STREQUAL "nil"))
        set(DIASDK_LIBRARIES "${path}/DIA SDK/lib")
        set(DIASDK_INCLUDE_DIR "${path}/DIA SDK/include")
    endif ()
else()
    if (CMAKE_SYSTEM_NAME MATCHES "Android")
        set(patch_header_dir ${CMAKE_CURRENT_SOURCE_DIR}/Android)
        set_property(GLOBAL PROPERTY PROJECT_DX_SHADER_COMPILER_PLATFORM_INCLUDE ${patch_header_dir})

        if (CMAKE_C_COMPILER_ID MATCHES "(Clang)|(GNU)")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${patch_header_dir}")
        endif ()
        if (CMAKE_CXX_COMPILER_ID MATCHES "(Clang)|(GNU)")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${patch_header_dir}")
        endif ()
    endif ()
endif ()

function(__detect_arch processor result)
    string(TOLOWER ${processor} val)
    if (val MATCHES "(amd)|(86)")
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            set(val "x64")
        elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            set(val "X86")
        endif ()
    elseif (val MATCHES "arm")
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            set(val "ARM64")
        elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            set(val "ARM")
        endif ()
    elseif (val MATCHES "aarch64")
        set(val "AArch64")
    endif ()
    set(${result} ${val} PARENT_SCOPE)
endfunction(__detect_arch)

__detect_arch(${CMAKE_SYSTEM_PROCESSOR} arch)
__detect_arch(${CMAKE_HOST_SYSTEM_PROCESSOR} host_arch)

__create_external_project_cmake_args(__CMAKE_ARGS)

set(predefined_config_params_file ${CMAKE_CURRENT_SOURCE_DIR}/predefined-config-params.cmake)
include(${predefined_config_params_file})
__get_predefined_config_params(predefined_params)

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/microsoft/DirectXShaderCompiler.git
    #GIT_TAG d0e9147ab86c8cb29a5fd81bd758e44d440c332c#release-1.5.1911
    GIT_TAG release-1.5.2002
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    GIT_CONFIG core.autocrlf=false core.eol=lf
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${AE_BUILD_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND
        ${CMAKE_COMMAND}
        -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/patch/patch.cmake
    BUILD_BYPRODUCTS 
        ${LLVMDxcSupport_SLIB_R}
        ${LLVMDxcSupport_SLIB_D}
    CMAKE_ARGS
        ${predefined_params}
        -DDXC_BUILD_ARCH=${arch}
        -DPREDEFINED_CONFIG_PARAMS_FILE=${predefined_config_params_file}
        -DDXC_BUILD_HOST_ARCH=${host_arch}
        #-DLLVM_ENABLE_WARNINGS:BOOL=OFF
        #-DLLVM_ENABLE_WERROR:BOOL=OFF
        -DDIASDK_LIBRARIES=${DIASDK_LIBRARIES}
        -DDIASDK_INCLUDE_DIR=${DIASDK_INCLUDE_DIR}
        #-DCMAKE_SHARED_LIBRARY_PREFIX=${CMAKE_SHARED_LIBRARY_PREFIX}
        #-DCMAKE_STATIC_LIBRARY_PREFIX=${CMAKE_STATIC_LIBRARY_PREFIX}
        ${__CMAKE_ARGS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${AE_EXTERNAL_FOLDER_NAME})