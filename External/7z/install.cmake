if (SRK_HOST_OS_WINDOWS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BUILD_DIR}/7zz.exe ${INSTALL_DIR}/7zz.exe)
endif ()