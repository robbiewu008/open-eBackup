::  This file is a part of the open-eBackup project.
::  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
::  If a copy of the MPL was not distributed with this file, You can obtain one at
::  http://mozilla.org/MPL/2.0/.
:: 
::  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
:: 
::  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
::  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
::  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.

@echo off
rem plugin_sdk.zip打包

rem 设置变量
@echo off

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set AGENTPATH=%CurBatPath%..\..\
SET INSTALLSHIELD_PACKAGE_DIR=%AGENTPATH%build\ms\package
set BUILD_LOG="%CurBatPath%"build.log

rem===================================================
echo %PATH% >> %BUILD_LOG%
rem unzip the open_src files
call %CurBatPath%agent_make_opensrc.bat

rem call 64 compile
call %CurBatPath%agent_make.bat compile64agent
if not "%errorlevel%" == "0" (
    echo "Exec %CurBatPath%agent_make.bat compile64agent fail."
    exit /b 1
)

SET AGENT_SDK_DIR=%AGENTPATH%plugin_sdk
SET SOURCE_FILE_DIR=%AGENTPATH%bin

IF EXIST %AGENT_SDK_DIR%                     RMDIR  /S/Q  %AGENT_SDK_DIR%
MKDIR  %AGENT_SDK_DIR%
MKDIR  %AGENT_SDK_DIR%\lib
MKDIR  %AGENT_SDK_DIR%\include
MKDIR  %AGENT_SDK_DIR%\include\common
MKDIR  %AGENT_SDK_DIR%\include\securec
MKDIR  %AGENT_SDK_DIR%\include\message
MKDIR  %AGENT_SDK_DIR%\include\message\archivestream

rem==================11. copy agent sdk begin======================
COPY /Y "%SOURCE_FILE_DIR%\agentsdk.dll"  %AGENT_SDK_DIR%\lib
COPY /Y "%SOURCE_FILE_DIR%\agentsdk.lib"  %AGENT_SDK_DIR%\lib
COPY /Y "%AGENTPATH%\src\inc\pluginfx\ExternalPluginSDK.h"  %AGENT_SDK_DIR%\include

COPY /Y "%AGENTPATH%\src\inc\common\Defines.h"  %AGENT_SDK_DIR%\include\common
COPY /Y "%AGENTPATH%\src\inc\common\Types.h"  %AGENT_SDK_DIR%\include\common
COPY /Y "%AGENTPATH%\platform\securec\include\securec.h"  %AGENT_SDK_DIR%\include\securec
COPY /Y "%AGENTPATH%\platform\securec\include\securectype.h"  %AGENT_SDK_DIR%\include\securec
COPY /Y "%AGENTPATH%\src\inc\message\archivestream\ArchiveStreamService.h"  %AGENT_SDK_DIR%\include\message\archivestream
rem==================11. copy agent sdk end========================

rem==================12. zip Agent begin========================
"%AGENTPATH%\third_party_groupware\7Zip\7z.exe" a "%INSTALLSHIELD_PACKAGE_DIR%\plugin_sdk.zip" "%AGENT_SDK_DIR%\include" "%AGENT_SDK_DIR%\lib"

echo  %DATE% %TIME% %1 zip Agent >> %BUILD_LOG%
rem==================12. zip Agent end==============================
echo Package plugin_sdk.zip success.
exit /b 0