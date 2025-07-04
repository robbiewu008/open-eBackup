@echo off
:: 
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
::

set AGENT_ROOT_PATH=%~1
set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set CURBAT_PATH=%~dp0
set NGINX_LOG_PATH=%CURBAT_PATH%nginx\logs\
set NGINX_EXE_PATH=%CURBAT_PATH%nginx\
set LOG_FILE_NAME="%AGENT_ROOT_PATH%\log\rotatenginxlog.log"

call :Log "Begin to rotate nginx log."

if exist "%NGINX_LOG_PATH%error.log.1.zip" (del /f /q "%NGINX_LOG_PATH%error.log.1.zip")
move "%NGINX_LOG_PATH%error.log" "%AGENT_ROOT_PATH%/tmp/error.log"

cd "%NGINX_EXE_PATH%"
.\rdnginx -s reopen >>"%CURBAT_PATH%..\log\monitor.log"

call :Log "Compress nginx log."
timeout /T 1 /NOBREAK >nul
call "%CURBAT_PATH%7z.exe" a -y -tzip "%NGINX_LOG_PATH%error.log.1.zip" "%AGENT_ROOT_PATH%/tmp/error.log" -mx=9 2>>"%CURBAT_PATH%..\log\monitor.log"

if exist "%AGENT_ROOT_PATH%/tmp/error.log" (del /f /q "%AGENT_ROOT_PATH%/tmp/error.log")
call :Log "Finish rotating nginx log."
exit 0
rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOG_FILE_NAME%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOG_FILE_NAME%
    goto :EOF
