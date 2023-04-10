srkSetSubProject("")

set(OUTPUT_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}dxcompiler)
set(LLVMDxcSupport ${CMAKE_STATIC_LIBRARY_PREFIX}LLVMDxcSupport)

set(LLVMDxcSupport_SLIB_PREFIX ${PROJECT_BINARY_DIR}/install/lib/${LLVMDxcSupport})
set(LLVMDxcSupport_SLIB_R ${LLVMDxcSupport_SLIB_PREFIX}${CMAKE_RELEASE_POSTFIX}${SRK_SLIB_SUFFIX})
set(LLVMDxcSupport_SLIB_D ${LLVMDxcSupport_SLIB_PREFIX}${CMAKE_DEBUG_POSTFIX}${SRK_SLIB_SUFFIX})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_INCLUDE ${PROJECT_BINARY_DIR}/install/include)
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_LLVMDxcSupport_SLIB_R ${LLVMDxcSupport_SLIB_R})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_LLVMDxcSupport_SLIB_D ${LLVMDxcSupport_SLIB_D})
set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_DLL ${PROJECT_BINARY_DIR}/install/${SRK_INSTALL_DLL_DIR_NAME}/${OUTPUT_NAME}${SRK_BUILD_TYPE_SUFFIX}${SRK_DLL_SUFFIX})

set(DIASDK_LIBRARIES "")
set(DIASDK_INCLUDE_DIR "")
if (SRK_CXX_COMPILER_MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wv:18")

    set(path "nil")
    srkFindParentDir(${CMAKE_CXX_COMPILER} path "Community" "Professional" "Enterprise")
    if (NOT (${path} STREQUAL "nil"))
        set(DIASDK_LIBRARIES "${path}/DIA SDK/lib")
        set(DIASDK_INCLUDE_DIR "${path}/DIA SDK/include")
    endif ()
else()
    if (SRK_OS_ANDROID)
        set(patch_header_dir ${CMAKE_CURRENT_SOURCE_DIR}/Android)
        set_property(GLOBAL PROPERTY SRKPROJ_EXTERNAL_DX_SHADER_COMPILER_PLATFORM_INCLUDE ${patch_header_dir})

        if (SRK_C_COMPILER_CLANG OR SRK_C_COMPILER_GCC)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${patch_header_dir}")
        endif ()
        if (SRK_CXX_COMPILER_CLANG OR SRK_CXX_COMPILER_GCC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${patch_header_dir}")
        endif ()
    endif ()
endif ()

function(_detect_arch archPrefix archWordBitsPrefix arch)
    if (${${archPrefix}X86})
        if (${${archWordBitsPrefix}32})
            set(${arch} "X86" PARENT_SCOPE)
        elseif (${${archWordBitsPrefix}64})
            set(${arch} "x64" PARENT_SCOPE)
        endif ()
    elseif (${${archPrefix}ARM})
        if (${${archWordBitsPrefix}32})
            set(${arch} "ARM" PARENT_SCOPE)
        elseif (${${archWordBitsPrefix}64})
            set(${arch} "ARM64" PARENT_SCOPE)
        endif ()
    endif ()
endfunction()
#AArch64

_detect_arch(SRK_ARCH_ SRK_ARCH_WORD_BITS_ arch)
_detect_arch(SRK_HOST_ARCH_ SRK_HOST_ARCH_WORD_BITS_ host_arch)

srkCreateExternalProjectCmakeArgs(cmakeArgs)

set(predefined_config_params_file ${CMAKE_CURRENT_SOURCE_DIR}/predefined-config-params.cmake)
include(${predefined_config_params_file})
__get_predefined_config_params(predefined_params)

set(gitTag v1.7.2212.1)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/microsoft/DirectXShaderCompiler.git
    GIT_TAG ${gitTag}
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    #GIT_CONFIG core.autocrlf=false core.eol=lf
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND
        ${CMAKE_COMMAND}
        -DSRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/src
        -DPATCH_DIR=${CMAKE_CURRENT_SOURCE_DIR}/patch
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
        -DHLSL_SUPPORT_QUERY_GIT_COMMIT_INFO=OFF
        -DLLVM_ENABLE_TIMESTAMPS=OFF
        -DLLVM_ENABLE_BACKTRACES=OFF
        -DLLVM_ENABLE_CRASH_OVERRIDES=OFF
        -DLLVM_BUILD_RUNTIME=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_DOCS=OFF
        #-DCMAKE_SHARED_LIBRARY_PREFIX=${CMAKE_SHARED_LIBRARY_PREFIX}
        #-DCMAKE_STATIC_LIBRARY_PREFIX=${CMAKE_STATIC_LIBRARY_PREFIX}
        ${cmakeArgs}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_EXTERNAL_DIR_NAME})