execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_LISTS_FILE} content)

string(REPLACE "set(PNG_LIB_NAME png\${PNGLIB_MAJOR}\${PNGLIB_MINOR})" "set(PNG_LIB_NAME png)" content "${content}")

string(REPLACE "find_program(AWK NAMES gawk awk)" "if(CMAKE_GENERATOR MATCHES \"Xcode\" AND CMAKE_XCODE_BUILD_SYSTEM MATCHES 12)\r\n    set(XCODE_NEW_BUILD_SYSTEM True)\r\nelse()\r\n  set(XCODE_NEW_BUILD_SYSTEM False)\r\n   find_program(AWK NAMES gawk awk)\r\nendif()" content "${content}")
string(REPLACE "if(NOT AWK OR ANDROID)" "if(XCODE_NEW_BUILD_SYSTEM OR NOT AWK OR ANDROID)" content "${content}")

string(REGEX REPLACE "# SET UP LINKS[\r\n]+if\\(PNG_SHARED" "if(False" content "${content}")
string(REPLACE "if(CYGWIN OR MINGW)" "if(False)" content "${content}")
string(REPLACE "if(NOT WIN32)" "if(False)" content "${content}")

file(WRITE ${CMAKE_LISTS_FILE} "${content}")