execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/GetHostTriple.cmake ${BIN_DIR}/src/cmake/modules)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/CrossCompile.cmake ${BIN_DIR}/src/cmake/modules)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/patch/TableGen.cmake ${BIN_DIR}/src/cmake/modules)