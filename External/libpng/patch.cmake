execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)

string(REPLACE "find_program(AWK NAMES gawk awk)" "if(CMAKE_GENERATOR MATCHES \"Xcode\" AND CMAKE_XCODE_BUILD_SYSTEM MATCHES 12)\r\n    set(XCODE_NEW_BUILD_SYSTEM True)\r\nelse()\r\n  set(XCODE_NEW_BUILD_SYSTEM False)\r\n   find_program(AWK NAMES gawk awk)\r\nendif()" content "${content}")
string(REPLACE "if(NOT AWK OR ANDROID)" "if(XCODE_NEW_BUILD_SYSTEM OR NOT AWK OR ANDROID)" content "${content}")

string(REPLACE "set(PNG_SHARED_OUTPUT_NAME \"png\${PNGLIB_ABI_VERSION}\")" "set(PNG_SHARED_OUTPUT_NAME png)" content "${content}")
string(REPLACE "set(PNG_STATIC_OUTPUT_NAME \"png\${PNGLIB_ABI_VERSION}\")" "set(PNG_STATIC_OUTPUT_NAME png)" content "${content}")
string(REPLACE "set(PNG_SHARED_OUTPUT_NAME \"libpng\${PNGLIB_ABI_VERSION}\")" "set(PNG_SHARED_OUTPUT_NAME libpng)" content "${content}")
string(REPLACE "set(PNG_STATIC_OUTPUT_NAME \"libpng\${PNGLIB_ABI_VERSION}_static\")" "set(PNG_STATIC_OUTPUT_NAME libpng_static)" content "${content}")
string(REPLACE "VERSION \"\${PNGLIB_SHARED_VERSION}\"" "" content "${content}")
string(REPLACE "SOVERSION \"\${PNGLIB_ABI_VERSION}\"" "" content "${content}")
string(REPLACE "if(CYGWIN OR MINGW)" "if(False)" content "${content}")
string(REPLACE "if(NOT WIN32)" "if(False)" content "${content}")

file(WRITE ${file} "${content}")