file(READ ${CMAKE_LISTS_FILE} content)
string(REPLACE "\"1.dll\"" "\"${AE_DLL_SUFFIX}\"" content ${content})
file(WRITE ${CMAKE_LISTS_FILE} ${content})