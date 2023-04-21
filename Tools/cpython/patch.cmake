if (SRK_HOST_OS_LINUX OR SRK_HOST_OS_MACOS)
    execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
    execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

    set(file ${SRC_DIR}/Modules/Setup)
    file(READ ${file} content)
    string(REPLACE "#zlib  zlibmodule.c -lz" "zlib  zlibmodule.c -I${ZLIB_INCLUDE} -L${ZLIB_DIR} -lz" content "${content}")
    file(WRITE ${file} "${content}")
endif ()