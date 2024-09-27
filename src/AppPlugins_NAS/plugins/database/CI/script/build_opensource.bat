@echo off
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set BUILD_LOG="%CurBatPath%CI_build.log"
set BASE_PATH=%CurBatPath%..\..\plugins\database\
echo BASE_PATH is %BASE_PATH% >> %BUILD_LOG%
set PLUGIN_BUILD_BAT=%BASE_PATH%build\build.bat

set FRAMEWORK_PATH=%BASE_PATH%..\..\framework
echo FRAMEWORK_PATH is %FRAMEWORK_PATH% >> %BUILD_LOG%
set FRAMEWORK_BUILD_BAT=%FRAMEWORK_PATH%\build\build.bat

xcopy %CurBatPath%\..\..\..\..\code\framework %CurBatPath%\..\..\framework\ /e/y
xcopy %CurBatPath%\..\..\..\..\code\vsprj %CurBatPath%\..\..\vsprj\ /e/y

call %FRAMEWORK_BUILD_BAT% OPENSOURCE
if %errorlevel% NEQ 0 (
    echo build framework failed! >> %BUILD_LOG%
    echo build framework failed!
    exit 1
)

call %PLUGIN_BUILD_BAT%
if %errorlevel% NEQ 0 (
    echo build databasePlugin failed! >> %BUILD_LOG%
    echo build databasePlugin failed!
    exit 1
)

echo Success! End of CI_Build. >> %BUILD_LOG%
echo Success! End of CI_Build.
endlocal
goto :eof