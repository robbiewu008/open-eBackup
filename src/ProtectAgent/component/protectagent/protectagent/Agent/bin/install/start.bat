@echo off
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
setlocal EnableDelayedExpansion

set CMD_PAUSE=pause

set DATA_BACKUP_AGENT_HOME_VAR=
for /f "tokens=2 delims==" %%a in ('wmic environment where "name='DATA_BACKUP_AGENT_HOME' and username='<system>'" get VariableValue /value') do (
    if not "%%a" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%%a
    )
)
if "%DATA_BACKUP_AGENT_HOME_VAR%" == "" (
    set DATA_BACKUP_AGENT_HOME_VAR=C:
)
set AGENT_BIN_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\ProtectClient\ProtectClient-E\bin
rem ---------------------------------------------------------------
rem                   Begin to start Agent
rem ---------------------------------------------------------------

net.exe session 1>NUL 2>NUL || (
    echo Please run the script as an administrator.
    %CMD_PAUSE%
    exit /b 1
)

:SelectStart
    if exist "%AGENT_BIN_PATH%" (
        call %AGENT_BIN_PATH%\agent_start.bat
        %CMD_PAUSE%
        exit /b 0
    ) else (
        call :gotoexit
    )
    goto :EOF

:gotoexit
    echo Not install any client,the startup will be stopped.
    %CMD_PAUSE%
    exit /b 1

endlocal