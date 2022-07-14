execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)
string(REGEX REPLACE "set_target_properties[^\r\n]+PROPERTIES[\r\n]+[^\r\n]*VERSION [^\r\n]+_SO_VERSION}[^\r\n]*[\r\n]+[^\r\n]*SOVERSION [^\r\n]+_SO_VERSION}\\)" "" content "${content}")
file(WRITE ${CMAKE_LISTS_FILE} "${content}")