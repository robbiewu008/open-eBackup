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
rem ########################################################################
rem # DBPlugin start.bat
rem ########################################################################

set LOG_PATH=%~1
set PORT_BEGIN=%~2
set PORT_END=%~3
set AGENT_IP=%4
set AGENT_PORT=%5

set PYTHON_PATH=%DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\install\Python310
set Path=%PYTHON_PATH%\;%PYTHON_PATH%\Scripts\;%Path%
set WORKON_HOME=%DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\install\virtualenvs
set START_LOG=%CurBatPath%start.log

setlocal EnableDelayedExpansion

call %PYTHON_PATH%\Scripts\workon.bat plugin_env
if %errorlevel% NEQ 0 (
    echo Enter virtual env failed! >> %START_LOG%
    exit /b 1
)
echo Enter virtual env success! >> %START_LOG%

start %DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\bin\GeneralDBPlugin.exe %LOG_PATH% %PORT_BEGIN% %PORT_END% %AGENT_IP% %AGENT_PORT%

endlocal
exit /b 0