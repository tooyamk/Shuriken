execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/GetHostTriple.cmake ${BIN_DIR}/src/cmake/modules)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/CrossCompile.cmake ${BIN_DIR}/src/cmake/modules)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/TableGen.cmake ${BIN_DIR}/src/cmake/modules)

set(cpp ${BIN_DIR}/src/lib/DxilRootSignature/DxilRootSignatureValidator.cpp)
file(READ ${cpp} content)
file(WRITE ${cpp} "#include <ios>\n${content}")