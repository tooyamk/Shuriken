execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(COPY ${SRC_DIR} DESTINATION ${DST_DIR})