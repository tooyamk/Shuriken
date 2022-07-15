execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/build/cmake/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "set (GLEW_LIB_NAME glew32)" "set (GLEW_LIB_NAME glew)" content "${content}")
string(REPLACE "set (GLEW_LIB_NAME GLEW)" "set (GLEW_LIB_NAME glew)" content "${content}")
file(WRITE ${file} "${content}")

file(COPY ${PATCH_DIR}/src/ DESTINATION ${SRC_DIR})