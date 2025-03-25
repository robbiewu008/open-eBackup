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

set NEED_LOG=1
set CURBAT_PATH=%~dp0
set SCRIPTNAME=%~nx0
set AGENT_BIN_PATH=%CURBAT_PATH%
set LOGFILENAME="%CURBAT_PATH%..\log\agent_stop.log"

set USAGE= Usage: "agent_stop" or "agent_stop rdagent" or "agent_stop monitor" or "agent_stop nginx" or "agent_stop provider"

if not "%2" == "" goto other
if "%1" == "rdagent" goto stopagent
if "%1" == "monitor" goto stopmonitor
if "%1" == "nginx" goto stopnginx

if "%1" == "" (
    call :stopmonitor
    call :stopnginx
    call :stopagent
)


exit /b 0

:stopagent
    set AGENT_CHECK=
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdagent.exe"') do (set AGENT_CHECK=%%i)

    echo !AGENT_CHECK! | findstr "rdagent.exe" >nul && (
        call taskkill /f /im rdagent.exe
    ) || (
        call :Log "Process rdagent of OceanStor BCManager Agent is not exist."
        echo Process rdagent of OceanStor BCManager Agent is not exist.
    )

    set AGENT_CHECK=
    timeout /T 3 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdagent.exe"') do (set AGENT_CHECK=%%i)

    echo !AGENT_CHECK! | findstr "rdagent.exe" >nul && (
        call :Log "Process rdagent of OceanStor BCManager Agent stop failed."
        echo Process rdagent of OceanStor BCManager Agent failed.
        exit 1
    ) || (
        call :Log "Process rdagent of OceanStor BCManager Agent stop successful."
        echo Process rdagent of OceanStor BCManager Agent stop successful.
    )
    goto :EOF
    
:stopmonitor
    set MONITOR_CHECK=
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq monitor.exe"') do (set MONITOR_CHECK=%%i)
    
    echo !MONITOR_CHECK! | findstr "monitor.exe" >nul && (
        call taskkill /f /im monitor.exe
    ) || (
        call :Log "Process monitor of OceanStor BCManager Agent is not exist."
        echo Process monitor of OceanStor BCManager Agent is not exist.
    )
    
    set MONITOR_CHECK=
    timeout /T 5 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq monitor.exe"') do (set MONITOR_CHECK=%%i)

    echo !MONITOR_CHECK! | findstr "monitor.exe" >nul && (
        call :Log "Process monitor of OceanStor BCManager Agent stop failed."
        echo Process monitor of OceanStor BCManager Agent stop failed.
        exit 1
    ) || (
        call :Log "Process monitor of OceanStor BCManager Agent stop successful."
        echo Process monitor of OceanStor BCManager Agent stop successful.
    )
    goto :EOF
    
:stopnginx
    set NGINX_CHECK=
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdnginx.exe"') do (set NGINX_CHECK=%%i)

    echo !NGINX_CHECK! | findstr "rdnginx.exe" >nul && (
        call taskkill /f /im rdnginx.exe
    ) || (
        call :Log "Process nginx of OceanStor BCManager Agent is not exist."
        echo Process nginx of OceanStor BCManager Agent is not exist.
    )
    
    rem waite 3 sec after query nginx
    timeout /T 3 /NOBREAK >nul
    set NGINX_CHECK=
    for /f "delims=" %%i in ('call tasklist /fi "Imagename eq rdnginx.exe"') do (set NGINX_CHECK=%%i)

    echo !NGINX_CHECK! | findstr "rdnginx.exe" >nul && (
        call :Log "Process nginx of OceanStor BCManager Agent stop failed."
        echo Process nginx of OceanStor BCManager Agent stop failed.
        exit 1
    ) || (
        call :Log "Process nginx of OceanStor BCManager Agent stop successful."
        echo Process nginx of OceanStor BCManager Agent stop successful.
    )
    goto :EOF
    
:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILENAME%
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILENAME%
    goto :EOF


:other
    echo %USAGE%
    
:end
    endlocal