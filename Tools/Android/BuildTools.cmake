#DESTINATION=

if (NOT DESTINATION)
    message(FATAL_ERROR "DESTINATION is not set")
endif ()

string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} value)
if (${value} MATCHES "(amd)|(86)")
    set(ARCH_X86 True)
elseif (${value} MATCHES "(arm)|(aarch)")
    set(ARCH_ARM True PARENT_SCOPE)
endif ()

cmake_host_system_information(RESULT IS_64Bit QUERY IS_64BIT)
if (${IS_64Bit} MATCHES "1")
    set(IS_64Bit True)
else ()
    set(IS_64Bit False)
endif ()

set(OpenJDKUrlBase https://builds.openlogic.com/downloadJDK/openlogic-openjdk/11.0.18+10/openlogic-openjdk-11.0.18+10-)
set(AndroidBuildToolsUrlBase https://dl.google.com/android/repository/build-tools_r33.0.2-)
set(AndroidNDKUrlBase https://dl.google.com/android/repository/android-ndk-r25c-)
set(AndroidPlatformToolsUrlBase https://dl.google.com/android/repository/platform-tools_r34.0.0-)
set(ninjaUrlBase https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-)

string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} os)
if (${os} MATCHES "windows")
    if (ARCH_X86)
        if (IS_64Bit)
            set(OpenJDKUrl ${OpenJDKUrlBase}windows-x64.zip)
        endif ()
    endif ()

    set(AndroidBuildToolsUrl ${AndroidBuildToolsUrlBase}windows.zip)
    set(AndroidNDKUrl ${AndroidNDKUrlBase}windows.zip)
    set(AndroidPlatformToolsUrl ${AndroidPlatformToolsUrlBase}windows.zip)

    set(ninjaUrl ${ninjaUrlBase}win.zip)
elseif (${os} MATCHES "linux")
    if (ARCH_X86)
        if (IS_64Bit)
            set(OpenJDKUrl ${OpenJDKUrlBase}linux-x64.tar.gz)
        endif ()
    endif ()

    set(AndroidBuildToolsUrl ${AndroidBuildToolsUrlBase}linux.zip)
    set(AndroidNDKUrl ${AndroidNDKUrlBase}linux.zip)
    set(AndroidPlatformToolsUrl ${AndroidPlatformToolsUrlBase}linux.zip)

    set(ninjaUrl ${ninjaUrlBase}linux.zip)
elseif (${os} MATCHES "darwin")
    if (ARCH_X86)
        if (IS_64Bit)
            set(OpenJDKUrl ${OpenJDKUrlBase}mac-x64.tar.gz)
        endif ()
    endif ()

    set(AndroidBuildToolsUrl ${AndroidBuildToolsUrlBase}macosx.zip)
    set(AndroidNDKUrl ${AndroidNDKUrlBase}macosx.zip)
    set(AndroidPlatformToolsUrl ${AndroidPlatformToolsUrlBase}macosx.zip)

    set(ninjaUrl ${ninjaUrlBase}mac.zip)
else ()
    message(FATAL_ERROR "not supported os")
endif ()

set(AndroidPlatforms
    platform-33 https://dl.google.com/android/repository/platform-33_r02.zip
)

if (NOT OpenJDKUrl)
    message(FATAL_ERROR "no OpenJDK url")
endif ()

if (NOT AndroidBuildToolsUrl)
    message(FATAL_ERROR "no Android build-tools url")
endif ()

if (NOT AndroidNDKUrl)
    message(FATAL_ERROR "no Android ndk url")
endif ()

if (NOT AndroidPlatformToolsUrl)
    message(FATAL_ERROR "no Android platform-tools url")
endif ()

if (NOT AndroidPlatforms)
    message(FATAL_ERROR "no Android platforms url")
endif ()

if (NOT ninjaUrl)
    message(FATAL_ERROR "no ninja url")
endif ()

set(break OFF)
while (NOT break)
    string(RANDOM rnd)
    set(tmp "${DESTINATION}/.tmp_${rnd}")
    if (NOT EXISTS ${tmp})
        set(break ON)
    endif ()
endwhile()

function(clean)
    file(REMOVE_RECURSE ${tmp}/)
endfunction()

file(MAKE_DIRECTORY ${tmp})

function(downloadAndExtract url dest stripComponents rmIfExist)
    get_filename_component(name ${url} NAME)
    set(tmpFile ${tmp}/${name})
    message("download ${url}")
    file(DOWNLOAD ${url} ${tmpFile} STATUS err SHOW_PROGRESS)
    if (NOT err EQUAL 0)
        clean()
        message(FATAL_ERROR "download file error ${url}")
    endif ()

    if (rmIfExist AND (EXISTS ${dest}))
        file(REMOVE_RECURSE ${dest}/)
    endif ()
    if (NOT EXISTS ${dest})
        file(MAKE_DIRECTORY ${dest})
    endif ()

    if (stripComponents)
        execute_process(COMMAND tar -xf ${tmpFile} -C ${dest} --strip-components 1)
    else ()
        file(ARCHIVE_EXTRACT INPUT ${tmpFile} DESTINATION ${dest})
    endif ()
endfunction()

file(MAKE_DIRECTORY ${tmp})

downloadAndExtract(${OpenJDKUrl} ${DESTINATION}/OpenJDK True True)
downloadAndExtract(${AndroidBuildToolsUrl} ${DESTINATION}/SDK/build-tools True True)
downloadAndExtract(${AndroidNDKUrl} ${DESTINATION}/SDK/ndk True True)
downloadAndExtract(${AndroidPlatformToolsUrl} ${DESTINATION}/SDK/platform-tools True True)

list(LENGTH AndroidPlatforms platformCount)
if (NOT platformCount EQUAL 0)
    MATH(EXPR platformCount "${platformCount}-1")
    foreach (i RANGE 0 ${platformCount} 2)
        list(GET AndroidPlatforms ${i} name)
        MATH(EXPR j "${i}+1")
        list(GET AndroidPlatforms ${j} url)

        downloadAndExtract(${url} ${DESTINATION}/SDK/platforms/${name} True True)
    endforeach ()
endif ()

downloadAndExtract(${ninjaUrl} ${DESTINATION} False False)

clean()