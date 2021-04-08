execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)
string(REPLACE "set(PNG_LIB_NAME png\${PNGLIB_MAJOR}\${PNGLIB_MINOR})" "set(PNG_LIB_NAME png)" content "${content}")
file(WRITE ${CMAKE_LISTS_FILE} "${content}")