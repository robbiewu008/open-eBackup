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
rem # DB RPC Tool
rem ########################################################################

set CUR_PATH=%~dp0
echo %CUR_PATH%
set INTERFACE_NAME=%~1
echo %INTERFACE_NAME%
set INPUT_FILE_NAME=%~2
echo %INPUT_FILE_NAME%
set OUTPUT_FILE_NAME=%~3
echo %INPUT_FILE_NAME%
set LOG_PATH=%CUR_PATH%..\..\..\ProtectClient-E\log\Plugins\GeneralDBPlugin
echo %LOG_PATH%
set THRIFT_SERVER_PORT_FILE_PATH=%CUR_PATH%..\..\..\ProtectClient-E\tmp\thriftserverport
echo %THRIFT_SERVER_PORT_FILE_PATH%
set /p THRIFT_SERVER_PORT=<%THRIFT_SERVER_PORT_FILE_PATH%
echo %THRIFT_SERVER_PORT%

%CUR_PATH%rpctool.exe %INTERFACE_NAME% %INPUT_FILE_NAME% %OUTPUT_FILE_NAME% %LOG_PATH% %THRIFT_SERVER_PORT%
if %errorlevel% equ 0 (
    echo Call dbrpctool success.
) else (
    echo Call dbrpctool failed.
    exit /b 1
)
exit /b 0