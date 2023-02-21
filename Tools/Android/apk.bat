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
    ) else if "%~1" == "PROJECT_LIB" (
        set PROJECT_LIB=%~2
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

:checkParams
if "%ANDROID_SDK_BUILD_TOOLS%" == "" (
    set /p ANDROID_SDK_BUILD_TOOLS=ANDROID_SDK_BUILD_TOOLS=
    goto:checkParams
)

set AAPT=%ANDROID_SDK_BUILD_TOOLS%/aapt.exe
if not exist "%AAPT%" (
    echo %AAPT% is not exist
    set /p ANDROID_SDK_BUILD_TOOLS=ANDROID_SDK_BUILD_TOOLS=
    goto:checkParams
)

set AAPT2=%ANDROID_SDK_BUILD_TOOLS%/aapt2.exe
if not exist "%AAPT2%" (
    echo %AAPT2% is not exist
    set /p ANDROID_SDK_BUILD_TOOLS=ANDROID_SDK_BUILD_TOOLS=
    goto:checkParams
)

set D8=%ANDROID_SDK_BUILD_TOOLS%/lib/d8.jar
if not exist "%D8%" (
    echo %D8% is not exist
    set /p ANDROID_SDK_BUILD_TOOLS=ANDROID_SDK_BUILD_TOOLS=
    goto:checkParams
)

if "%ANDROID_SDK_PLATFORM%" == "" (
    echo ANDROID_SDK_PLATFORM is not set
    set /p ANDROID_SDK_PLATFORM=ANDROID_SDK_PLATFORM=
    goto:checkParams
)

set ANDROID_JAR=%ANDROID_SDK_PLATFORM%/android.jar
if not exist "%ANDROID_JAR%" (
    echo %ANDROID_JAR% is not exist
    set /p ANDROID_SDK_PLATFORM=ANDROID_SDK_PLATFORM=
    goto:checkParams
)

if "%JDK%" == "" (
    set /p JDK=JDK=
    goto:checkParams
)

set JAVA=%JDK%/bin/java.exe
if not exist "%JAVA%" (
    echo %JAVA% is not exist
    set /p JDK=JDK=
    goto:checkParams
)

set JAVAC=%JDK%/bin/javac.exe
if not exist "%JAVA%" (
    echo %JAVAC% is not exist
    set /p JDK=JDK=
    goto:checkParams
)

set JARSIGNER=%JDK%/bin/jarsigner.exe
if not exist "%JARSIGNER%" (
    echo %JARSIGNER% is not exist
    set /p JDK=JDK=
    goto:checkParams
)

if defined PROJECT_RES (
    if not exist "%PROJECT_RES%" (
        echo %PROJECT_RES% is not exist
        set /p PROJECT_RES=PROJECT_RES=
        goto:checkParams
    )
)

if defined PROJECT_LIBS (
    if not exist "%PROJECT_LIBS%" (
        echo %PROJECT_LIBS% is not exist
        set /p PROJECT_LIBS=PROJECT_LIBS=
        goto:checkParams
    )
)

if "%MANIFEST%" == "" (
    set /p MANIFEST=MANIFEST=
    goto:checkParams
)

if not exist "%MANIFEST%" (
    echo %MANIFEST% is not exist
    set /p MANIFEST=MANIFEST=
    goto:checkParams
)

if "%KEYSTORE%" == "" (
    set /p KEYSTORE=KEYSTORE=
    goto:checkParams
)

if not exist "%KEYSTORE%" (
    echo %KEYSTORE% is not exist
    set /p KEYSTORE=KEYSTORE=
    goto:checkParams
)

if "%KEY_ALIAS%" == "" (
    set /p KEY_ALIAS=KEY_ALIAS=
    goto:checkParams
)

if "%BUILD_DIR%" == "" (
    set /p BUILD_DIR=BUILD_DIR=
    goto:checkParams
)

if "%BUILD_APK_NAME%" == "" (
    set /p BUILD_APK_NAME=BUILD_APK_NAME=
    goto:checkParams
)

if not exist "%BUILD_DIR%" md "%BUILD_DIR%"

:initTmpDir
set tmp=%BUILD_DIR%/.tmp_%random%
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
    if exist "%PROJECT_RES%" (
        "%AAPT2%" compile --dir "%PROJECT_RES%" -o "%tmpRes%"
    )
)

set flatFiles=
for %%i in (%tmpRes%/*.flat) do (
    set flatFiles=!flatFiles! "%tmpRes%/%%~ni%%~xi"
)

set apkUnsign=%tmp%/app.apk
set apk=%BUILD_DIR%/%BUILD_APK_NAME%.apk
"%AAPT2%" link -I "%ANDROID_JAR%" %flatFiles% --java "%tmpJava%" --manifest "%MANIFEST%" -o "%apkUnsign%"

set javaFiles=
call:depthCollectFiles %tmpJava% %tmpJava% java javaFiles
"%JAVAC%" -d "%tmpClass%" -cp "%ANDROID_JAR%" %javaFiles%

set classFiles=
call:depthCollectFiles %tmpClass% %tmpClass% class classFiles
"%JAVA%" -cp "%D8%" com.android.tools.r8.D8 %classFiles% --output "%tmp%"

set oriDir=%~dp0

cd /d "%tmp%"
"%AAPT%" a "%apkUnsign%" classes.dex

if defined PROJECT_LIB (
    if exist "%PROJECT_LIB%" (
        cd /d "%PROJECT_LIB%/.."
        for /f "delims=" %%i in ("%PROJECT_LIB%") do set libs=%%~ni
        call:depthCollectFiles %PROJECT_LIB% !libs! so soFiles
        "%AAPT%" a "%apkUnsign%" !soFiles!
    )
)

cd /d "%oriDir%"

"%JARSIGNER%" -keystore "%KEYSTORE%" -signedjar "%apk%" "%apkUnsign%" %KEY_ALIAS%

goto:clean

:depthCollectFiles
call:depthCollectFiles_f %~1 %~2 %~3 %~4
call:depthCollectFiles_d %~1 %~2 %~3 %~4
goto:eof

:depthCollectFiles_d
for /d %%i in (%~1/*) do (
    call:depthCollectFiles_f %~1/%%~ni %~2/%%~ni %~3 %~4
    call:depthCollectFiles_d %~1/%%~ni %~2/%%~ni %~3 %~4
)
goto:eof

:depthCollectFiles_f
for %%i in (%~1/*.%~3) do (
    set "%~4=!%~4! "%~2/%%~ni%%~xi""
)
goto:eof

:clean
rmdir /q /s "%tmp%"