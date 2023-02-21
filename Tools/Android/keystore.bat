echo off

:initParamsWhile
if not "%~1" == "" (
    if "%~1" == "JDK" (
        set JDK=%~2
    ) else if "%~1" == "ALIAS" (
        set ALIAS=%~2
    ) else if "%~1" == "PASSWORD" (
        set PASSWORD=%~2
    ) else if "%~1" == "OUTPUT" (
        set OUTPUT=%~2
    )

    shift
    shift
    goto:initParamsWhile
)

:checkParams
if "%JDK%" == "" (
    set /p JDK=JDK=
    goto:checkParams
)

set KEY_TOOL=%JDK%/bin/keytool.exe
if not exist "%KEY_TOOL%" (
    echo %KEY_TOOL% is not exist
    set /p JDK=JDK=
    goto:checkParams
)

if "%OUTPUT%" == "" (
    set /p OUTPUT=OUTPUT=
    goto:checkParams
)

if "%ALIAS%" == "" set ALIAS=shuriken
if "%PASSWORD%" == "" set PASSWORD=111111

"%KEY_TOOL%" -genkeypair -alias %ALIAS% -keypass %PASSWORD% -storepass %PASSWORD% -dname "C=unknown,ST=unknown,L=unknown,O=unknown,OU=unknown,CN=unknown" -keyalg RSA -keystore "%OUTPUT%"