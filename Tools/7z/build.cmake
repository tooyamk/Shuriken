file(REMOVE_RECURSE ${INSTALL_DIR})
file(MAKE_DIRECTORY ${INSTALL_DIR})

if (SRK_CXX_COMPILER_MSVC)
    if (NOT (${VCVARSALL} STREQUAL "nil"))
        set(arch "nil")
        if (SRK_HOST_ARCH_X86)
            if (SRK_HOST_ARCH_WORD_BITS_32)
                set(arch "x86")
            elseif (SRK_HOST_ARCH_WORD_BITS_64)
                set(arch "x64")
            endif ()
        elseif (SRK_HOST_ARCH_ARM)
            if (SRK_HOST_ARCH_WORD_BITS_32)
                set(arch "arm")
            elseif (SRK_HOST_ARCH_WORD_BITS_64)
                set(arch "arm64")
            endif ()
        endif ()

        if (NOT (${arch} STREQUAL "nil"))
            execute_process(COMMAND ${CMAKE_CUR_SRC_DIR}/build.bat ${VCVARSALL} ${arch} WORKING_DIRECTORY ${PROJ_DIR} RESULT_VARIABLE err)
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJ_DIR}/${arch}/7zz.exe ${INSTALL_DIR})
            file(REMOVE_RECURSE ${PROJ_DIR}/${arch})
        endif ()
    endif ()
elseif (SRK_HOST_OS_MACOS)
    execute_process(COMMAND make -f makefile.gcc CFLAGS_WARN=-Wno-unused-but-set-variable WORKING_DIRECTORY ${PROJ_DIR} RESULT_VARIABLE err)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJ_DIR}/_o/7zz ${INSTALL_DIR})
    file(REMOVE_RECURSE ${PROJ_DIR}/_o)
endif ()