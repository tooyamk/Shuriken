echo off

:initParamsWhile
if not "%~1" == "" (
    if "%~1" == "BUILD_DIR" (
        set BUILD_DIR=%~2
    ) else if "%~1" == "NINJA" (
        set NINJA=%~2
    ) else if "%~1" == "ANDROID_NDK" (
        set ANDROID_NDK=%~2
    ) else if "%~1" == "ANDROID_ABI" (
        set ANDROID_ABI=%~2
    ) else if "%~1" == "SRK_ROOT" (
        set SRK_ROOT=%~2
    )

    shift
    shift
    goto:initParamsWhile
)

if "%BUILD_DIR%" == "" (
    echo BUILD_DIR is not set
    exit /b 1
)

if "%NINJA%" == "" (
    echo NINJA is not set
    exit /b 1
)

if not exist "%NINJA%" (
    echo Ninja executable file is not exist
    exit /b 1
)

if "%ANDROID_NDK%" == "" (
    echo ANDROID_NDK is not set
    exit /b 1
)

if not exist "%ANDROID_NDK%" (
    echo ANDROID_NDK dir is not exist
    exit /b 1
)

if "%ANDROID_ABI%" == "" (
    echo ANDROID_ABI is not set
    exit /b 1
)

if "%SRK_ROOT%" == "" (
    echo SRK_ROOT is not set
    exit /b 1
)

set CMAKE_LISTS_FILE=%SRK_ROOT%/CMakeLists.txt
if not exist "%CMAKE_LISTS_FILE%" (
    echo %CMAKE_LISTS_FILE% is not exist
    exit /b 1
)

call:configure %BUILD_DIR% Debug
call:configure %BUILD_DIR% Release
goto:end

:configure
set buildType=%~2
set buildDir=%~1/%buildType%
del /f /q "%buildDir%\CMakeCache.txt"
echo on
cmake -DSRK_ENABLE_TESTS=OFF -DSRK_ENABLE_EXTERNAL_ZSTD=ON -DSRK_ENABLE_EXTERNAL_DX_SHADER_COMPILER=ON -DCMAKE_INSTALL_PREFIX=install -DCMAKE_CXX_FLAGS="-D__cpp_lib_remove_cvref -D__cpp_lib_bitops -DSRK_std_convertible_to -DSRK_std_default_initializable -DSRK_std_derived_from -DSRK_std_floating_point -DSRK_std_integral -DSRK_std_invocable -DSRK_std_signed_integral -DSRK_std_unsigned_integral -UANDROID -llog" -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK%/build/cmake/android.toolchain.cmake" -DANDROID_ABI=%ANDROID_ABI% -DANDROID_NDK="%ANDROID_NDK%" -DANDROID_PLATFORM=android-22 -DCMAKE_BUILD_TYPE=%buildType% -DCMAKE_MAKE_PROGRAM="%NINJA%" -B "%buildDir%" "%SRK_ROOT%" -G Ninja
echo off
goto:eof

:end