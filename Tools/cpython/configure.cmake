if (SRK_HOST_OS_LINUX OR SRK_HOST_OS_MACOS)
message("${ZLIB_INCLUDE}          ${ZLIB_DIR}")
    execute_process(COMMAND ${SRC_DIR}/configure --enable-optimizations --prefix=${INSTALL_DIR} CPPFLAGS="\$CPPFLAGS -I${ZLIB_INCLUDE}" LDFLAGS="\$LDFLAGS -Wl,-rpath ${ZLIB_DIR}" WORKING_DIRECTORY ${BUILD_DIR} RESULT_VARIABLE err)
endif ()