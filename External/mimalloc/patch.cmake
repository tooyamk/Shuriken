execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)
string(REPLACE "set(mi_basename \"\${mi_basename}-\${CMAKE_BUILD_TYPE_LC}\")" "set(mi_basename \"\${mi_basename}\")" content "${content}")
file(WRITE ${CMAKE_LISTS_FILE} "${content}")