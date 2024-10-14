@echo off
setlocal EnableDelayedExpansion

set CurBatPath=%~dp0
set WORKSPACE=%CurBatPath%../../../
set BUILD_PKG_TYPE=%~1
set BUILD_OS_TYPE=%~2

call ci_build_dir.bat %BUILD_PKG_TYPE% %BUILD_OS_TYPE%

if not exist "%WORKSPACE%temp" (
    md  "%WORKSPACE%temp"
)

xcopy /y /e "%WORKSPACE%../../../../open-source-obligation/dependency/Windows/*" "%WORKSPACE%temp"

