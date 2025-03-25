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
setlocal EnableDelayedExpansion

set CURBAT_PATH=%~dp0
set SCRIPTNAME=%~nx0
set NEED_LOG=1
set AGENT_BIN_PATH=%CURBAT_PATH%
set LOGFILENAME="%CURBAT_PATH%\..\log\agent_start.log"
set AGENT_SERVICE_NAME=rdagent
set NGINX_SERVICE_NAME=rdnginx
set WORKING_USER_NAME=rdadmin
set DS_REPAIR_MODE=DsRepair

set USAGE= Usage: "agent_start" or "agent_start rdagent" or "agent_start monitor" or "agent_start nginx" or "agent_start provider"

call :checkServiceUser

if not "%2" == "" goto other
if "%1" == "nginx" goto startnginx
if "%1" == "rdagent" goto startagent
if "%1" == "monitor" goto startmonitor

if "%1" == "" (
    call :startnginx
    call :startagent
    call :startmonitor
)

exit /b 0

:checkServiceUser
    set USER_NAME=
    for /f "tokens=2 delims='='" %%a in ('wmic service where "name='%NGINX_SERVICE_NAME%'" get startname /value') do (
        set USER_NAME=%%a
    )
    set USER_NAME=%USER_NAME:.\=%

    call :checkBootMode

    if "!USER_NAME!" == "%WORKING_USER_NAME%" (
        if "!STATUS!" == "%DS_REPAIR_MODE%" (
            call "%AGENT_BIN_PATH%\agent_stop.bat" %AGENT_SERVICE_NAME%
            call "%AGENT_BIN_PATH%\agent_start.bat" %AGENT_SERVICE_NAME%
            call "%AGENT_BIN_PATH%\agent_stop.bat" %NGINX_SERVICE_NAME% 
            call "%AGENT_BIN_PATH%\agent_start.bat" %NGINX_SERVICE_NAME%
        )
    ) else (
        if "!STATUS!" == ""  (
            call "%AGENT_BIN_PATH%\agent_stop.bat" %AGENT_SERVICE_NAME%
            call "%AGENT_BIN_PATH%\agent_start.bat" %AGENT_SERVICE_NAME%
            call "%AGENT_BIN_PATH%\agent_stop.bat" %NGINX_SERVICE_NAME%
            call "%AGENT_BIN_PATH%\agent_start.bat" %NGINX_SERVICE_NAME%
        ) else (
            for /f "delims=" %%i in ('2^>nul sc query !NGINX_SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
            if "!SERVICE_CHECK!" == "" (
                call :Log "Nginx start!"
                call "%AGENT_BIN_PATH%\agent_start.bat" %NGINX_SERVICE_NAME%
            ) 
        )
    )
    goto :EOF

:checkBootMode
    set STATUS=
    for /f "tokens=2 delims=: " %%a in ('bcdedit') do (
        if "%%a"=="%DS_REPAIR_MODE%" (
            set STATUS=%DS_REPAIR_MODE%
        )
    )
    goto :EOF

:startagent
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdagent.exe"') do (set AGENT_CHECK=%%i)
    
    echo !AGENT_CHECK! | findstr "rdagent.exe" >nul && (
        call :Log "Process rdagent of OceanStor BCManager Agent is exist."
        echo Process rdagent of OceanStor BCManager Agent is exist.
    ) || (
        start /b rdagent.exe
    )
    
    set AGENT_CHECK=
    timeout /T 1 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdagent.exe"') do (set AGENT_CHECK=%%i)

    echo !AGENT_CHECK! | findstr "rdagent.exe" >nul && (
        call :Log "Process rdagent of OceanStor BCManager Agent successful."
        echo Process rdagent of OceanStor BCManager Agent start successful.
    ) || (
        call :Log "Process rdagent of OceanStor BCManager Agent start failed."
        echo Process rdagent of OceanStor BCManager Agent start failed.
        exit 1
    )
    goto :EOF
    
:startmonitor
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq monitor.exe"') do (set MONITOR_CHECK=%%i)
    
    echo !MONITOR_CHECK! | findstr "monitor.exe" >nul && (
        call :Log "Process monitor of OceanStor BCManager Agent is exist."
        echo Process monitor of OceanStor BCManager Agent is exist.
    ) || (
        start /b monitor.exe
    )
    
    set MONITOR_CHECK=
    timeout /T 1 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq monitor.exe"') do (set MONITOR_CHECK=%%i)

    echo !MONITOR_CHECK! | findstr "monitor.exe" >nul && (
        call :Log "Process monitor of OceanStor BCManager Agent start successful."
        echo Process monitor of OceanStor BCManager Agent start successful.
    ) || (
        call :Log "Process monitor of OceanStor BCManager Agent start failed."
        echo Process monitor of OceanStor BCManager Agent start failed.
        exit 1
    )
    goto :EOF
    
:startnginx
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdnginx.exe"') do (set NGINX_CHECK=%%i)
    
    echo !NGINX_CHECK! | findstr "rdnginx.exe" >nul && (
        call :Log "Process nginx of OceanStor BCManager Agent is exist."
        echo Process nginx of OceanStor BCManager Agent is exist.
        goto :EOF
    ) || (
        cd /d "%CURBAT_PATH%\..\nginx"
        start /b rdnginx.exe
    )
    
    timeout /T 3 /NOBREAK >nul
    set NGINX_CHECK=
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdnginx.exe"') do (set NGINX_CHECK=%%i)

    echo !NGINX_CHECK! | findstr "rdnginx.exe" >nul && (
        call :Log "Process nginx of OceanStor BCManager Agent start successful."
        echo Process nginx of OceanStor BCManager Agent start successful.
    ) || (
        call :Log "Process nginx of OceanStor BCManager Agent start failed."
        echo Process nginx of OceanStor BCManager Agent start failed.
        exit 1
    )
    
    goto :EOF

:other
    echo %USAGE%

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILENAME%
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILENAME%
    goto :EOF

:end
    endlocal