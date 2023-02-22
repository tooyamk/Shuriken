echo off

:initParamsWhile
if not "%~1" == "" (
    if "%~1" == "BUILD_DIR" (
        set BUILD_DIR=%~2
    ) else if "%~1" == "NINJA" (
        set NINJA=%~2
    ) else if "%~1" == "ANDROID_NDK" (
        set ANDROID_NDK=%~2
    ) else if "%~1" == "ANDROID_PLATFORM" (
        set ANDROID_PLATFORM=%~2
    ) else if "%~1" == "ANDROID_ABI" (
        set ANDROID_ABI=%~2
    ) else if "%~1" == "SRK_ROOT" (
        set SRK_ROOT=%~2
    )

    shift
    shift
    goto:initParamsWhile
)

:checkParams
if "%BUILD_DIR%" == "" (
    set /p BUILD_DIR=BUILD_DIR=
    goto:checkParams
)

if "%NINJA%" == "" (
    set /p NINJA=NINJA=
    goto:checkParams
)

if not exist "%NINJA%" (
    echo Ninja executable file is not exist
    set /p NINJA=NINJA=
    goto:checkParams
)

if "%ANDROID_NDK%" == "" (
    set /p ANDROID_NDK=ANDROID_NDK=
    goto:checkParams
)

if not exist "%ANDROID_NDK%" (
    echo ANDROID_NDK dir is not exist
    set /p ANDROID_NDK=ANDROID_NDK=
    goto:checkParams
)

if "%ANDROID_PLATFORM%" == "" (
    set /p ANDROID_PLATFORM=ANDROID_PLATFORM=
    goto:checkParams
)

if "%ANDROID_ABI%" == "" (
    set /p ANDROID_ABI=ANDROID_ABI=
    goto:checkParams
)

if "%SRK_ROOT%" == "" (
    set /p SRK_ROOT=SRK_ROOT=
    goto:checkParams
)

set CMAKE_LISTS_FILE=%SRK_ROOT%/CMakeLists.txt
if not exist "%CMAKE_LISTS_FILE%" (
    echo %CMAKE_LISTS_FILE% is not exist
    set /p SRK_ROOT=SRK_ROOT=
    goto:checkParams
)

call:configure %BUILD_DIR% %ANDROID_ABI% Debug
call:configure %BUILD_DIR% %ANDROID_ABI% Release
goto:end

:configure
set abi=%~2
set buildType=%~3
set buildDir=%~1/%abi%-%buildType%
del /f /q "%buildDir%\CMakeCache.txt"
echo on
cmake -DSRK_ENABLE_TESTS=ON -DSRK_ENABLE_EXTERNAL_ZSTD=ON -DCMAKE_INSTALL_PREFIX=install -DCMAKE_CXX_FLAGS="-D__cpp_lib_remove_cvref -D__cpp_lib_bitops -DSRK_std_convertible_to -DSRK_std_default_initializable -DSRK_std_derived_from -DSRK_std_floating_point -DSRK_std_integral -DSRK_std_invocable -DSRK_std_signed_integral -DSRK_std_unsigned_integral -UANDROID" -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK%/build/cmake/android.toolchain.cmake" -DANDROID_ABI=%abi% -DANDROID_NDK="%ANDROID_NDK%" -DANDROID_PLATFORM=%ANDROID_PLATFORM% -DCMAKE_BUILD_TYPE=%buildType% -DCMAKE_MAKE_PROGRAM="%NINJA%" -B "%buildDir%" "%SRK_ROOT%" -G Ninja
echo off
goto:eof

:end