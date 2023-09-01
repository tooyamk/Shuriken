execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/build/cmake/lib/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "set_source_files_properties(\${Sources} PROPERTIES LANGUAGE C)" "if(NOT CMAKE_ASM_COMPILER STREQUAL CMAKE_C_COMPILER)\r\nset_source_files_properties(\${Sources} PROPERTIES LANGUAGE C)\r\nendif()" content "${content}")
string(REPLACE "VERSION \${zstd_VERSION_MAJOR}.\${zstd_VERSION_MINOR}.\${zstd_VERSION_PATCH}" "" content "${content}")
string(REPLACE "SOVERSION \${zstd_VERSION_MAJOR}" "" content "${content}")
file(WRITE ${file} "${content}")