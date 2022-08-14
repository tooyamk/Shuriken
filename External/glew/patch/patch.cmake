execute_process(COMMAND git -C ${PROJ_BIN_DIR}/src clean -xfd)
execute_process(COMMAND git -C ${PROJ_BIN_DIR}/src reset --hard)

set(file ${PROJ_BIN_DIR}/src/build/cmake/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "set (GLEW_LIB_NAME glew32)" "set (GLEW_LIB_NAME glew)" content "${content}")
string(REPLACE "set (GLEW_LIB_NAME GLEW)" "set (GLEW_LIB_NAME glew)" content "${content}")
file(WRITE ${file} "${content}")

file(ARCHIVE_EXTRACT INPUT ${CMAKE_CUR_SRC_DIR}/patch/files.zip DESTINATION ${PROJ_BIN_DIR}/tmp)
file(COPY ${PROJ_BIN_DIR}/tmp/src/ DESTINATION ${PROJ_BIN_DIR}/src)
file(REMOVE_RECURSE INPUT ${PROJ_BIN_DIR}/tmp/src)