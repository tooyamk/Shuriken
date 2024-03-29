#SRK_ROOT=
#LIBS=<abi>dir|<abi>dir|...
#OUTPUT=output file path

####can search from SRK_ANDROID_BUILD_TOOLS
#JDK=
#ANDROID_SDK_BUILD_TOOLS=
#ANDROID_SDK_PLATFORM=

#[SRK_ANDROID_BUILD_TOOLS]=
#[EXCLUDE_LIBS]=libname1|libname2|...
#[APP_NAME]=
#[LIB_NAME]=
#[KEYSTORE]=
#[KEY_ALIAS]=
#[STORE_PASS]=

if (NOT SRK_ROOT)
    message(FATAL_ERROR "SRK_ROOT is not set")
endif ()

if (NOT LIBS)
    message(FATAL_ERROR "LIBS is not set")
endif ()

if (NOT OUTPUT)
    message(FATAL_ERROR "OUTPUT is not set")
endif ()

if ((NOT JDk) AND SRK_ANDROID_BUILD_TOOLS)
    set(JDK ${SRK_ANDROID_BUILD_TOOLS}/OpenJDK)
endif ()
if (NOT JDK)
    message(FATAL_ERROR "JDK is not set")
endif ()

if ((NOT ANDROID_SDK_BUILD_TOOLS) AND SRK_ANDROID_BUILD_TOOLS)
    set(ANDROID_SDK_BUILD_TOOLS ${SRK_ANDROID_BUILD_TOOLS}/SDK/build-tools)
endif ()
if (NOT ANDROID_SDK_BUILD_TOOLS)
    message(FATAL_ERROR "ANDROID_SDK_BUILD_TOOLS is not set")
endif ()

if ((NOT ANDROID_SDK_PLATFORM) AND SRK_ANDROID_BUILD_TOOLS)
    file(GLOB platforms "${SRK_ANDROID_BUILD_TOOLS}/SDK/platforms/*")
    set(platformVer 0)
    foreach(platform ${platforms})
        if (IS_DIRECTORY ${platform})
            get_filename_component(name ${platform} NAME)
            string(REPLACE "platform-" "" ver ${name})
            if (ANDROID_SDK_PLATFORM)
                if(${ver} GREATER ${platformVer})
                    set(ANDROID_SDK_PLATFORM ${platform})
                    set(platformVer ${ver})
                endif ()
            else ()
                set(ANDROID_SDK_PLATFORM ${platform})
                set(platformVer ${ver})
            endif ()
        endif ()
    endforeach()
endif ()
if (NOT ANDROID_SDK_PLATFORM)
    message(FATAL_ERROR "ANDROID_SDK_PLATFORM is not set")
endif ()

if (EXCLUDE_LIBS)
    string(REPLACE "|" ";" EXCLUDE_LIBS ${EXCLUDE_LIBS})
else ()
    set(EXCLUDE_LIBS "")
endif ()

set(java ${JDK}/bin/java)
set(javac ${JDK}/bin/javac)
set(jarsigner ${JDK}/bin/jarsigner)
set(keytool ${JDK}/bin/keytool)
set(aapt ${ANDROID_SDK_BUILD_TOOLS}/aapt)
set(aapt2 ${ANDROID_SDK_BUILD_TOOLS}/aapt2)
set(d8 ${ANDROID_SDK_BUILD_TOOLS}/lib/d8.jar)
set(android_jar ${ANDROID_SDK_PLATFORM}/android.jar)

if (WIN32)
    set(java ${java}.exe)
    set(javac ${javac}.exe)
    set(jarsigner ${jarsigner}.exe)
    set(keytool ${keytool}.exe)
    set(aapt ${aapt}.exe)
    set(aapt2 ${aapt2}.exe)
endif ()

function(copyFiles from to matchExt)
    file(GLOB files ${from}/*)
    foreach (f ${files})
        if (IS_DIRECTORY ${f})
            get_filename_component(n ${f} NAME)
            string(SUBSTRING ${n} 0 1 s)
            if(NOT ${s} STREQUAL ".")
                copyFiles(${f} ${to}/${n} ${matchExt})
            endif ()
        else ()
            get_filename_component(e ${f} LAST_EXT)
            if ("${e}" STREQUAL ${matchExt})
                get_filename_component(n ${f} NAME)
                set(exclude False)
                foreach(excludeLib ${EXCLUDE_LIBS})
                    if ("${n}" STREQUAL "${excludeLib}")
                        set(exclude True)
                        break()
                    endif ()
                endforeach()
                
                if (NOT exclude)
                    file(COPY ${f} DESTINATION ${to})
                endif ()
            endif ()
        endif ()
    endforeach ()
endfunction()

get_filename_component(dir ${OUTPUT} DIRECTORY)

#=================================
set(break OFF)
while (NOT break)
    string(RANDOM rnd)
    set(tmp "${dir}/.tmp_${rnd}")
    if (NOT EXISTS ${tmp})
        set(break ON)
    endif ()
endwhile()

file(MAKE_DIRECTORY ${tmp}/app)
file(MAKE_DIRECTORY ${tmp}/app/lib)
file(MAKE_DIRECTORY ${tmp}/build)

#=================================
string(REPLACE "|" ";" arr ${LIBS})
foreach (i ${arr})
    string(LENGTH ${i} len)
    math(EXPR len "${len} - 1")
    string(SUBSTRING ${i} 1 ${len} i)
    string(REPLACE ">" ";" pair ${i})
    list(GET pair 0 abi)
    list(GET pair 1 path)

    copyFiles(${path} ${tmp}/app/lib/${abi} ".so")
endforeach ()

#=================================
file(COPY ${SRK_ROOT}/Tools/Android/Apk/res DESTINATION ${tmp}/app)
set(file ${tmp}/app/res/values/strings.xml)
file(READ ${file} content)
if (APP_NAME)
    string(REPLACE ">app_name<" ">${APP_NAME}<" content "${content}")
endif ()
if (LIB_NAME)
    string(REPLACE ">lib_name<" ">${LIB_NAME}<" content "${content}")
endif ()
file(WRITE ${file} "${content}")

#=================================
file(MAKE_DIRECTORY ${tmp}/build/flat)
execute_process(COMMAND ${aapt2} compile --dir ${tmp}/app/res -o ${tmp}/build/flat)

#=================================
file(MAKE_DIRECTORY ${tmp}/build/java)
file(GLOB_RECURSE files "${tmp}/build/flat/*.flat")
set(cmd COMMAND ${aapt2} link -I ${android_jar})
foreach (i ${files})
    list(APPEND cmd ${i})
endforeach ()
list(APPEND cmd --java ${tmp}/build/java --manifest ${SRK_ROOT}/Tools/Android/apk/AndroidManifest.xml -o ${tmp}/build/app.apk)
execute_process(${cmd})

#=================================
file(MAKE_DIRECTORY ${tmp}/build/class)
file(GLOB_RECURSE files "${tmp}/build/java/*.java")
set(cmd COMMAND ${javac} -d ${tmp}/build/class -cp ${android_jar})
foreach (i ${files})
    list(APPEND cmd ${i})
endforeach ()
execute_process(${cmd})

#=================================
file(GLOB_RECURSE files "${tmp}/build/class/*.class")
set(cmd COMMAND ${java} -cp ${d8} com.android.tools.r8.D8)
foreach (i ${files})
    list(APPEND cmd ${i})
endforeach ()
list(APPEND cmd --output ${tmp}/build)
execute_process(${cmd})

#=================================
execute_process(COMMAND ${aapt} a ${tmp}/build/app.apk classes.dex WORKING_DIRECTORY ${tmp}/build)

function(packLibs from relativePath)
    file(GLOB files ${from}/*)
    foreach (f ${files})
        get_filename_component(n ${f} NAME)
        if (IS_DIRECTORY ${f})
            packLibs(${f} ${relativePath}/${n})
        else ()
            execute_process(COMMAND ${aapt} a ${tmp}/build/app.apk ${relativePath}/${n} WORKING_DIRECTORY ${tmp}/app)
        endif ()
    endforeach ()
endfunction()

packLibs(${tmp}/app/lib lib)

#=================================
if (NOT KEYSTORE)
    set(KEYSTORE ${tmp}/build/srk.keystore)
    if (NOT KEY_ALIAS)
        set(KEY_ALIAS shuriken)
    endif ()
    if (NOT STORE_PASS)
        set(STORE_PASS 111111)
    endif ()
    execute_process(COMMAND ${keytool} -genkey -alias ${KEY_ALIAS} -storepass ${STORE_PASS} -dname "C=unknown,ST=unknown,L=unknown,O=unknown,OU=unknown,CN=unknown" -keyalg RSA -keystore ${KEYSTORE} --storetype PKCS12)
endif ()

#=================================
set(cmd COMMAND ${jarsigner} -keystore ${KEYSTORE} -signedjar ${OUTPUT} ${tmp}/build/app.apk ${KEY_ALIAS})
if (STORE_PASS)
    list(APPEND cmd -storepass ${STORE_PASS})
endif ()
execute_process(${cmd})

#=================================
file(REMOVE_RECURSE ${tmp}/)