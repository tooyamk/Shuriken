execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "set(mi_basename \"\${mi_basename}-\${CMAKE_BUILD_TYPE_LC}\")" "set(mi_basename \"\${mi_basename}\")" content "${content}")
string(REPLACE "VERSION \${mi_version} SOVERSION \${mi_version_major}" "" content "${content}")
file(WRITE ${file} "${content}")