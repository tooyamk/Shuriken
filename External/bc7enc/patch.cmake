execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)
string(REGEX REPLACE "option\(BUILD_X64.*add_executable\(bc7enc \$\{BC7ENC_SRC_LIST\}\)" "add_library(bc7enc STATIC bc7enc.h bc7enc.c)" content "${content}")
string(REGEX REPLACE "if \\(NOT MSVC\\).*$" "" content "${content}")
set(content "${content}\r\ninstall(FILES bc7enc.h DESTINATION include)")
set(content "${content}\r\ninstall(TARGETS bc7enc RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)")
file(WRITE ${file} "${content}")