execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/CMakeLists.txt ${BIN_DIR}/src)

set(def ${BIN_DIR}/src/libusb/libusb-1.0.def)

file(READ ${def} content)
string(REPLACE "\"libusb-1.0.dll\"" "" content ${content})
file(WRITE ${def} ${content})