file(REMOVE_RECURSE ${INSTALL_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR}/include ${INSTALL_DIR}/include)