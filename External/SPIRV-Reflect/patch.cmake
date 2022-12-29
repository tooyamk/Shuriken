file(REMOVE_RECURSE ${INSTALL_DIR})
file(INSTALL ${SRC_DIR}/spirv_reflect.h DESTINATION ${INSTALL_DIR}/include)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR}/include ${INSTALL_DIR}/include/include)