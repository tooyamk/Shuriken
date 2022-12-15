file(REMOVE_RECURSE ${INSTALL_DIR})

if (NOT (${SRC_DIR} STREQUAL nil))
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR} ${INSTALL_DIR}/bin)
endif ()