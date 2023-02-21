echo off

:initParamsWhile
if not "%~1" == "" (
    if "%~1" == "SDK_ROOT" (
        set SDK_ROOT=%~2
    )

    shift
    shift
    goto:initParamsWhile
)

:checkParams
if "%SDK_ROOT%" == "" (
    set /p SDK_ROOT=SDK_ROOT=
    goto:checkParams
)

:initTmpDir
set tmp=%SDK_ROOT%/.tmp_%random%
if exist "%tmp%" (
    goto:initTmpDir
) else (
    md "%tmp%"
)

if not exist "%SDK_ROOT%" md "%SDK_ROOT%"

if not exist "%SDK_ROOT%/build-tools" md "%SDK_ROOT%/build-tools"
curl -o "%tmp%/build-tools.zip" https://dl.google.com/android/repository/build-tools_r33.0.2-windows.zip
tar -xf "%tmp%/build-tools.zip" -C "%SDK_ROOT%/build-tools" --strip-components 1

if not exist "%SDK_ROOT%/ndk" md "%SDK_ROOT%/ndk"
curl -o "%tmp%/ndk.zip" https://dl.google.com/android/repository/android-ndk-r25c-windows.zip
tar -xf "%tmp%/ndk.zip" -C "%SDK_ROOT%/ndk" --strip-components 1

curl -o "%tmp%/platform-tools.zip" https://dl.google.com/android/repository/platform-tools_r34.0.0-windows.zip
tar -xf "%tmp%/platform-tools.zip" -C "%SDK_ROOT%"

if not exist "%SDK_ROOT%/platforms/platform-33" md "%SDK_ROOT%/platforms/platform-33"
curl -o "%tmp%/platform-33.zip" https://dl.google.com/android/repository/platform-33_r02.zip
tar -xf "%tmp%/platform-33.zip" -C "%SDK_ROOT%/platforms/platform-33" --strip-components 1

rmdir /q /s "%tmp%"