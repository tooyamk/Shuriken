execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)

string(REPLACE "include(cmakescripts/GNUInstallDirs.cmake)" "include(cmakescripts/GNUInstallDirs.cmake)\r\nset(CMAKE_INSTALL_LIBDIR lib)" content "${content}")
string(REGEX REPLACE "set_target_properties[^\r\n]+[\r\n]+[     ]+SOVERSION \\$\\{TURBOJPEG_SO_MAJOR_VERSION} VERSION \\$\\{TURBOJPEG_SO_VERSION}\\)" "" content "${content}")

file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/sharedlib/CMakeLists.txt)
file(READ ${file} content)

string(REPLACE "set_target_properties(jpeg PROPERTIES SOVERSION \${SO_MAJOR_VERSION}" "" content "${content}")
string(REPLACE "VERSION \${SO_MAJOR_VERSION}.\${SO_AGE}.\${SO_MINOR_VERSION})" "" content "${content}")
string(REPLACE "RUNTIME_OUTPUT_NAME jpeg\${SO_MAJOR_VERSION})" "RUNTIME_OUTPUT_NAME jpeg)" content "${content}")
string(REPLACE "set_target_properties(jpeg PROPERTIES SUFFIX -\${SO_MAJOR_VERSION}.dll)" "" content "${content}")

file(WRITE ${file} "${content}")