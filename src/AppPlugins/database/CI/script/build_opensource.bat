rem
rem This file is a part of the open-eBackup project.
rem This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
rem If a copy of the MPL was not distributed with this file, You can obtain one at
rem http://mozilla.org/MPL/2.0/.
rem
rem Copyright (c) [2024] Huawei Technologies Co.,Ltd.
rem
rem THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
rem EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
rem MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
rem
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