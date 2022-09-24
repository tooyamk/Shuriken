execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/Source/cmake_core.cmake)
file(READ ${file} content)
string(REPLACE "if\(\${CLI}\)" "install\(TARGETS \${ASTC_TARGET}-static DESTINATION \${PACKAGE_ROOT}\)\n\nif\(\${CLI}\)" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/Source/astcenc_find_best_partitioning.cpp)
file(READ ${file} content)
file(WRITE ${file} "#include <limits>\r\n${content}")