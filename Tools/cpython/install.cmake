if (SRK_HOST_OS_WINDOWS)
    file(WRITE ${INSTALL_DIR}/bin/python${MAJOR}${MINOR}._pth "python${MAJOR}${MINOR}.zip\r\n.\r\nimport site")
    file(WRITE ${INSTALL_DIR}/bin/Lib/site-packages/start_path.pth "import sys;import os;sys.path.insert(0,os.path.dirname(sys.argv[0]));")
elseif (SRK_HOST_OS_LINUX OR SRK_HOST_OS_MACOS)
    execute_process(COMMAND make install WORKING_DIRECTORY ${BUILD_DIR} RESULT_VARIABLE err)
endif ()

