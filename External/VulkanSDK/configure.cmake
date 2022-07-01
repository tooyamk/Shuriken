file(REMOVE_RECURSE ${SRC_DIR})

if (SRK_OS_WINDOWS)
    execute_process(COMMAND ${7Z_EXE} x ${FILE_PATH} -o${SRC_DIR})  
endif ()