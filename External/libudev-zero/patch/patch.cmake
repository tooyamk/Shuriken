execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

file(COPY ${PATCH_DIR}/CMakeLists.txt DESTINATION ${SRC_DIR}/)