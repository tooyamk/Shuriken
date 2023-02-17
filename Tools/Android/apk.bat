echo off
setlocal enabledelayedexpansion

:initParamsWhile
if not "%~1" == "" (
    if "%~1" == "ANDROID_SDK_BUILD_TOOLS" (
        set ANDROID_SDK_BUILD_TOOLS=%~2
    ) else if "%~1" == "ANDROID_SDK_PLATFORM" (
        set ANDROID_SDK_PLATFORM=%~2
    ) else if "%~1" == "JDK" (
        set JDK=%~2
    ) else if "%~1" == "PROJECT_RES" (
        set PROJECT_RES=%~2
    ) else if "%~1" == "MANIFEST" (
        set MANIFEST=%~2
    ) else if "%~1" == "KEYSTORE" (
        set KEYSTORE=%~2
    ) else if "%~1" == "KEY_ALIAS" (
        set KEY_ALIAS=%~2
    ) else if "%~1" == "BUILD_DIR" (
        set BUILD_DIR=%~2
    ) else if "%~1" == "BUILD_APK_NAME" (
        set BUILD_APK_NAME=%~2
    ) 

    shift
    shift
    goto:initParamsWhile
)

if "%ANDROID_SDK_BUILD_TOOLS%" == "" (
    echo ANDROID_SDK_BUILD_TOOLS is not set
    exit /b 1
)

set AAPT=%ANDROID_SDK_BUILD_TOOLS%/aapt.exe
if not exist "%AAPT%" (
    echo %AAPT% is not exist
    exit /b 1
)

set AAPT2=%ANDROID_SDK_BUILD_TOOLS%/aapt2.exe
if not exist "%AAPT2%" (
    echo %AAPT2% is not exist
    exit /b 1
)

set D8=%ANDROID_SDK_BUILD_TOOLS%/lib/d8.jar
if not exist "%D8%" (
    echo %D8% is not exist
    exit /b 1
)

if "%ANDROID_SDK_PLATFORM%" == "" (
    echo ANDROID_SDK_PLATFORM is not set
    exit /b 1
)

set ANDROID_JAR=%ANDROID_SDK_PLATFORM%/android.jar
if not exist "%ANDROID_JAR%" (
    echo %ANDROID_JAR% is not exist
    exit /b 1
)

if "%JDK%" == "" (
    echo JDK is not set
    exit /b 1
)

set JAVA=%JDK%/bin/java.exe
if not exist "%JAVA%" (
    echo %JAVA% is not exist
    exit /b 1
)

set JAVAC=%JDK%/bin/javac.exe
if not exist "%JAVA%" (
    echo %JAVAC% is not exist
    exit /b 1
)

set JARSIGNER=%JDK%/bin/jarsigner.exe
if not exist "%JARSIGNER%" (
    echo %JARSIGNER% is not exist
    exit /b 1
)

if defined PROJECT_RES (
    if not exist "%PROJECT_RES%" (
        echo %PROJECT_RES% is not exist
        exit /b 1
    )
)

if "%MANIFEST%" == "" (
    echo MANIFEST is not set
    exit /b 1
)

if not exist "%MANIFEST%" (
    echo %MANIFEST% is not exist
    exit /b 1
)

if "%KEYSTORE%" == "" (
    echo KEYSTORE is not set
    exit /b 1
)

if not exist "%KEYSTORE%" (
    echo %KEYSTORE% is not exist
    exit /b 1
)

if "%KEY_ALIAS%" == "" (
    echo KEY_ALIAS is not set
    exit /b 1
)

if "%BUILD_DIR%" == "" (
    echo BUILD_DIR is not set
    exit /b 1
)

if "%BUILD_APK_NAME%" == "" (
    echo BUILD_APK_NAME is not set
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    md "%BUILD_DIR%"
)

:initTmpDir
set tmp=%BUILD_DIR%/tmp_%random%
if exist "%tmp%" (
    goto:initTmpDir
) else (
    md "%tmp%"
)

set tmpRes=%tmp%/res
md "%tmpRes%"
set tmpJava=%tmp%/java
md "%tmpJava%"
set tmpClass=%tmp%/class
md "%tmpClass%"
   
if defined PROJECT_RES (
    "%AAPT2%" compile --dir "%PROJECT_RES%" -o "%tmpRes%"
)

set flatFiles=
for %%i in (%tmpRes%/*.flat) do (
    set flatFiles=!flatFiles! "%tmpRes%/%%~ni%%~xi"
)

set apkUnsign=%tmp%/app.apk
set apk=%BUILD_DIR%/%BUILD_APK_NAME%.apk
"%AAPT2%" link -I "%ANDROID_JAR%" %flatFiles% --java "%tmpJava%" --manifest "%MANIFEST%" -o "%apkUnsign%"

set javaFiles=
call:depthCollectFiles %tmpJava% java javaFiles
"%JAVAC%" -d "%tmpClass%" -cp "%ANDROID_JAR%" %javaFiles%

set classFiles=
call:depthCollectFiles %tmpClass% class classFiles
"%JAVA%" -cp "%D8%" com.android.tools.r8.D8 %classFiles% --output "%tmp%"

set oriDir=%~dp0
cd /d "%tmp%"
"%AAPT%" a "%apkUnsign%" classes.dex
cd /d "%oriDir%"

"%JARSIGNER%" -keystore "%KEYSTORE%" -signedjar "%apk%" "%apkUnsign%" %KEY_ALIAS%

goto:clean

:depthCollectFiles
for /d %%i in (%~1/*) do (
    call:depthCollectFiles %~1/%%~ni %~2 %~3
    call:depthCollectFiles_f %~1/%%~ni %~2 %~3
)
goto:eof

:depthCollectFiles_f
for %%i in (%~1/*.%~2) do (
    set "%~3=!%~3! "%~1/%%~ni%%~xi""
)
goto:eof

:clean
rmdir /q /s "%tmp%"