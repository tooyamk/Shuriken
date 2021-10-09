execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_CORE_FILE} content)
string(REPLACE "if\(\${CLI}\)" "install\(TARGETS \${ASTC_TARGET}-static DESTINATION \${PACKAGE_ROOT}\)\n\nif\(\${CLI}\)" content "${content}")
file(WRITE ${CMAKE_CORE_FILE} "${content}")