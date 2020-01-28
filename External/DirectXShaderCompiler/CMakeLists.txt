__set_project()
set_property(GLOBAL PROPERTY __PROJECT_DX_SHADER_COMPILER_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY __PROJECT_DX_SHADER_COMPILER_INCLUDE ${PROJECT_BINARY_DIR}/src/include)
set_property(GLOBAL PROPERTY __PROJECT_DX_SHADER_COMPILER_DLL ${PROJECT_BINARY_DIR}/install/bin/dxcompiler$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX})

include(ExternalProject)

function(__findDir in out names)
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
endfunction(__findDir)

set(DIASDK_LIBRARIES "")
set(DIASDK_INCLUDE_DIR "")
if (MSVC)
    set(cxxFlags "${CMAKE_CXX_FLAGS} /Wv:18")

    set(path "nil")
    __findDir(${CMAKE_CXX_COMPILER} path "Community" "Professional" "Enterprise")
    if (NOT (${path} STREQUAL "nil"))
    message(${path})
        set(DIASDK_LIBRARIES "${path}/DIA SDK/lib")
        set(DIASDK_INCLUDE_DIR "${path}/DIA SDK/include")
    endif ()
else ()
    set(cxxFlags "${CMAKE_CXX_FLAGS}")
endif ()

string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} arch)
string(REGEX MATCH "(.*amd.*)|(.*x86.*)" rst ${arch})
string(LENGTH "${rst}" len)
if (${len} GREATER 0)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(arch "x64")
    elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
        set(arch "Win32")
    else ()
        set(arch "")
    endif ()
else ()
    string(REGEX MATCH ".*arm.*" rst ${arch})
    string(LENGTH "${rst}" len)
    if (${len} GREATER 0)
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            set(arch "ARM64")
        elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            set(arch "ARM")
        else ()
            set(arch "")
        endif ()
    else ()
        set(arch "")
    endif ()
endif ()

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/microsoft/DirectXShaderCompiler.git
    #GIT_TAG v1.4.1907
    #GIT_TAG v1.2.0-alpha
    GIT_TAG d0e9147ab86c8cb29a5fd81bd758e44d440c332c#release-1.5.1911
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${__BUILD_DIR}
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_CXX_FLAGS=${cxxFlags}
        -DBUILD_SHARED_LIBS=OFF
        -DLLVM_BUILD_EXAMPLES=OFF
        -DLLVM_BUILD_TESTS=OFF
        -DLLVM_BUILD_DOCS=OFF
        -DSPIRV_BUILD_TESTS=OFF
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
        -DSPIRV_TOOLS_EXTRA_DEFINITIONS="/D_ITERATOR_DEBUG_LEVEL=0"
        -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_WERROR=OFF
        -DINSTALL_GTEST=OFF
        -DBUILD_GMOCK=OFF
        -Dgtest_force_shared_crt=ON
        -DSPIRV_SKIP_EXECUTABLES=ON
        -DENABLE_SPIRV_CODEGEN=ON
        -DCLANG_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_TESTS=OFF
        -DHLSL_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_DOCS=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DLIBCLANG_BUILD_STATIC=ON
        -DLLVM_OPTIMIZED_TABLEGEN=OFF
        -DLLVM_REQUIRES_EH=ON
        -DLLVM_APPEND_VC_REV=ON
        -DLLVM_ENABLE_RTTI=ON
        -DLLVM_ENABLE_EH=ON
        -DLLVM_ENABLE_TERMINFO=OFF
        -DCLANG_BUILD_EXAMPLES=OFF
        -DLLVM_REQUIRES_RTTI=ON
        -DSPIRV_BUILD_TESTS=OFF
        -DSPIRV_SKIP_TESTS=ON
        -DCLANG_ENABLE_STATIC_ANALYZER=OFF
        -DCLANG_ENABLE_ARCMT=OFF
        -DCLANG_CL=OFF
        -DLLVM_TARGETS_TO_BUILD:STRING=None
        -DDXC_BUILD_ARCH:STRING=${arch}
        -DLLVM_DEFAULT_TARGET_TRIPLE:STRING=dxil-ms-dx
        #-DLLVM_ENABLE_WARNINGS:BOOL=OFF
        #-DLLVM_ENABLE_WERROR:BOOL=OFF
        -DDIASDK_LIBRARIES:STRING=${DIASDK_LIBRARIES}
        -DDIASDK_INCLUDE_DIR:STRING=${DIASDK_INCLUDE_DIR}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${__AE_EXTERNAL_FOLDER_NAME})