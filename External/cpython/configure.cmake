if (SRK_HOST_OS_LINUX OR SRK_HOST_OS_MACOS)
    execute_process(COMMAND ${SRC_DIR}/configure --enable-optimizations --prefix=${INSTALL_DIR} WORKING_DIRECTORY ${BUILD_DIR} RESULT_VARIABLE err)
endif ()