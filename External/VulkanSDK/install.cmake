file(REMOVE_RECURSE ${INSTALL_DIR})

if (NOT (${SRC_DIR} STREQUAL nil))
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR}/include ${INSTALL_DIR}/include)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR}/lib ${INSTALL_DIR}/lib)
endif ()