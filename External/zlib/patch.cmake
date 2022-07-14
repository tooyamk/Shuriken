execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)
string(REPLACE "\"1.dll\"" "\"${SRK_DLL_SUFFIX}\"" content "${content}")
string(REPLACE "set_target_properties(zlib PROPERTIES SOVERSION 1)" "" content "${content}")
string(REPLACE "set_target_properties(zlib PROPERTIES VERSION \${ZLIB_FULL_VERSION})" "" content "${content}")
file(WRITE ${CMAKE_LISTS_FILE} "${content}")