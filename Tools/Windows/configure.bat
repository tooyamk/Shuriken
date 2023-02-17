echo off

:initParamsWhile
if not "%~1" == "" (
    if "%~1" == "BUILD_DIR" (
        set BUILD_DIR=%~2
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

if "%SRK_ROOT%" == "" (
    echo SRK_ROOT is not set
    exit /b 1
)

set CMAKE_LISTS_FILE=%SRK_ROOT%/CMakeLists.txt
if not exist "%CMAKE_LISTS_FILE%" (
    echo %CMAKE_LISTS_FILE% is not exist
    exit /b 1
)

del /f /q "%BUILD_DIR%\CMakeCache.txt"
echo on
cmake -DSRK_ENABLE_TESTS=ON -DCMAKE_INSTALL_PREFIX=install -B "%BUILD_DIR%" "%SRK_ROOT%"