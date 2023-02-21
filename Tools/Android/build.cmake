#LIBS=<abi>dir|<abi>dir|...
#OUTPUT=output file path
message("=====================")

if (WIN32)

else ()
endif ()

get_filename_component(dir ${OUTPUT} DIRECTORY)

set(break OFF)
while (NOT break)
    string(RANDOM rnd)
    set(tmp ${dir}/.tmp_${rnd})
    if (NOT EXISTS ${tmp})
        set(break ON)
    endif ()
endwhile()

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${tmp}/lib)

string(REPLACE "|" ";" arr ${LIBS})
foreach (i ${arr})
    message("${i}")
    string(LENGTH ${i} len)
    math(EXPR len "${len} - 1")
    string(SUBSTRING ${i} 1 ${len} i)
    string(REPLACE ">" ";" pair ${i})
    list(GET pair 0 abi)
    list(GET pair 1 path)

    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${tmp}/lib/${abi})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${path} ${tmp}/lib/${abi})
    message("${abi}      ${path}            ${tmp}/lib/${abi}")
endforeach ()