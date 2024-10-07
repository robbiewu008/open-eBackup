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
rem # FilePlugin start.bat
rem ########################################################################

set LOG_PATH=%~1
set PORT_BEGIN=%~2
set PORT_END=%~3
set AGENT_IP=%4
set AGENT_PORT=%5

set CurBatPath=%~dp0
set START_LOG=%LOG_PATH%start.log

setlocal EnableDelayedExpansion

start %CurBatPath%bin\VirtualizationPlugin.exe %LOG_PATH% %PORT_BEGIN% %PORT_END% %AGENT_IP% %AGENT_PORT%

endlocal
exit 0