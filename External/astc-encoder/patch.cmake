execute_process(COMMAND git -C ${GIT_ROOT} clean -xfd)
execute_process(COMMAND git -C ${GIT_ROOT} reset --hard)

file(READ ${CMAKE_CORE_FILE} content)
string(REPLACE "add_executable\(astcenc-\${ISA_SIMD}\)" "add_library\(astcenc-\${ISA_SIMD} STATIC\)" content ${content})
string(REPLACE "astcenccli_toplevel_help.cpp\)" "astcenccli_toplevel_help.cpp\nastcenc_lib.h\nastcenc_lib.cpp\)" content ${content})
string(REPLACE "\$<\$<CXX_COMPILER_ID:MSVC>:/EHsc>" "\$<\$<CXX_COMPILER_ID:MSVC>:/EHsc>\n\$<\$<CXX_COMPILER_ID:MSVC>:\$<IF:\$<CONFIG:Debug>,/MDd,/MD>>" content ${content})
file(WRITE ${CMAKE_CORE_FILE} ${content})

file(COPY ${SRC_DIR} DESTINATION ${DST_DIR})