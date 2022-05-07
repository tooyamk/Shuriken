execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)
string(REPLACE "\"1.dll\"" "\"${SRK_DLL_SUFFIX}\"" content "${content}")
file(WRITE ${CMAKE_LISTS_FILE} "${content}")