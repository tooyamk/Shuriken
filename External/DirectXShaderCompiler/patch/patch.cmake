execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/GetHostTriple.cmake ${BIN_DIR}/src/cmake/modules)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/CrossCompile.cmake ${BIN_DIR}/src/cmake/modules)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/TableGen.cmake ${BIN_DIR}/src/cmake/modules)

set(cpp ${BIN_DIR}/src/lib/DxilRootSignature/DxilRootSignatureValidator.cpp)
file(READ ${cpp} content)
file(WRITE ${cpp} "#include <ios>\n${content}")

set(cpp ${BIN_DIR}/src/external/SPIRV-Tools/source/val/validate.cpp)
file(READ ${cpp} content)
string(REPLACE "for (const auto " "for (const auto& " content "${content}")
file(WRITE ${cpp} "${content}")

set(cpp ${BIN_DIR}/src/external/SPIRV-Tools/source/val/validation_state.cpp)
file(READ ${cpp} content)
string(REPLACE "for (const Function " "for (const Function& " content "${content}")
file(WRITE ${cpp} "${content}")