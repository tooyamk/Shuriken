execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)
string(REPLACE "set (GLEW_LIB_NAME glew32)" "set (GLEW_LIB_NAME glew)" content "${content}")
string(REPLACE "set (GLEW_LIB_NAME GLEW)" "set (GLEW_LIB_NAME glew)" content "${content}")
file(WRITE ${CMAKE_LISTS_FILE} "${content}")

file(COPY ${SRC_DIR} DESTINATION ${DST_DIR})