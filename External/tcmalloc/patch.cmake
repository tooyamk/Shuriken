execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)
string(REGEX REPLACE "set_target_properties[^\r\n]+PROPERTIES[\r\n]+[^\r\n]*VERSION [^\r\n]+_SO_VERSION}[^\r\n]*[\r\n]+[^\r\n]*SOVERSION [^\r\n]+_SO_VERSION}\\)" "" content "${content}")
file(WRITE ${file} "${content}")