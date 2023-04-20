#BUILD_DIR=
#ANDROID_ABI=
#SRK_ROOT=

####can search from SRK_ANDROID_BUILD_TOOLS
#NINJA=
#ANDROID_NDK=
#ANDROID_PLATFORM=

#[SRK_ANDROID_BUILD_TOOLS]=
#[BUILD_TYPES]=

if (NOT BUILD_DIR)
    message(FATAL_ERROR "BUILD_DIR is not set")
endif ()

if (NOT ANDROID_ABI)
    message(FATAL_ERROR "ANDROID_ABI is not set")
endif ()

if (NOT SRK_ROOT)
    message(FATAL_ERROR "SRK_ROOT is not set")
endif ()

if ((NOT NINJA) AND SRK_ANDROID_BUILD_TOOLS)
    set(NINJA ${SRK_ANDROID_BUILD_TOOLS}/ninja)
    if (WIN32)
        set(NINJA ${NINJA}.exe)
    endif ()
endif ()
if (NOT NINJA)
    message(FATAL_ERROR "NINJA is not set")
endif ()

if ((NOT ANDROID_NDK) AND SRK_ANDROID_BUILD_TOOLS)
    set(ANDROID_NDK ${SRK_ANDROID_BUILD_TOOLS}/SDK/ndk)
endif ()
if (NOT ANDROID_NDK)
    message(FATAL_ERROR "ANDROID_NDK is not set")
endif ()

if ((NOT ANDROID_PLATFORM) AND SRK_ANDROID_BUILD_TOOLS)
    file(GLOB platforms "${SRK_ANDROID_BUILD_TOOLS}/SDK/platforms/*")
    set(platformVer 0)
    foreach(platform ${platforms})
        if (IS_DIRECTORY ${platform})
            get_filename_component(name ${platform} NAME)
            string(REPLACE "platform-" "" ver ${name})
            if (ANDROID_PLATFORM)
                if(${ver} GREATER ${platformVer})
                    set(ANDROID_PLATFORM android-${ver})
                    set(platformVer ${ver})
                endif ()
            else ()
                set(ANDROID_PLATFORM android-${ver})
                set(platformVer ${ver})
            endif ()
        endif ()
    endforeach()
endif ()
if (NOT ANDROID_PLATFORM)
    message(FATAL_ERROR "ANDROID_PLATFORM is not set")
endif ()

if (BUILD_TYPES)
    string(REPLACE "|" ";" BUILD_TYPES ${BUILD_TYPES})
else ()
    set(BUILD_TYPES Debug Release)
endif ()

set(cxxFlags "-D__cpp_lib_remove_cvref -D__cpp_lib_bitops -DSRK_std_convertible_to -DSRK_std_default_initializable -DSRK_std_derived_from -DSRK_std_floating_point -DSRK_std_integral -DSRK_std_invocable -DSRK_std_signed_integral -DSRK_std_unsigned_integral -UANDROID")

foreach(buildType ${BUILD_TYPES})
    if ((${buildType} STREQUAL "Debug") OR (${buildType} STREQUAL "Release"))
        set(buildDir ${BUILD_DIR}/${ANDROID_ABI}/${buildType})
        file(REMOVE ${buildDir}/CMakeCache.txt)
        execute_process(COMMAND ${CMAKE_COMMAND} -DSRK_ENABLE_TESTS=ON -DSRK_ENABLE_EXTERNAL_ZSTD=ON -DCMAKE_INSTALL_PREFIX=install -DCMAKE_CXX_FLAGS=${cxxFlags} 
            -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake -DANDROID_ABI=${ANDROID_ABI} -DANDROID_NDK=${ANDROID_NDK} -DANDROID_PLATFORM=${ANDROID_PLATFORM}
            -DCMAKE_BUILD_TYPE=${buildType} -DCMAKE_MAKE_PROGRAM=${NINJA} -G Ninja -B ${buildDir} ${SRK_ROOT})
    endif ()
endforeach()