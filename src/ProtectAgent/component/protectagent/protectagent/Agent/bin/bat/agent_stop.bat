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
::The script is used to start Agent Service.

set CURRENT=%~dp0
set AGENT_ROOT=%CURRENT%..\
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%log\
set LOGFILE_PATH="%AGENT_LOG_PATH%agent_stop.log"
set NEED_LOG=1

echo Begin stop OceanStor BCManager Agent services...
echo.

rem stop rdmonitor service
call :stoptservice rdmonitor
if not "!errorlevel!" == "0" (exit /b 1)

rem stop rdagent service
call :stoptservice rdagent
if not "!errorlevel!" == "0" (exit /b 1)

rem stop rdnginx service
call :stoptservice rdnginx
if not "!errorlevel!" == "0" (exit /b 1)

rem stop rdprovider service
call :stoptservice rdprovider
if not "!errorlevel!" == "0" (exit /b 1)

echo.
echo OceanStor BCManager Agent services has been successfully stopped.

timeout /T 3 /NOBREAK >nul
call :Log "OceanStor BCManager Agent services has been successfully stopped."

goto :end

:stoptservice
    set SERVICE_CHECK=
    set SERVICE_NAME=%~1
    set RETRY_COUNT=1
    
    call :checkservice !SERVICE_NAME!
    if not "!errorlevel!" == "0" (
        echo The service !SERVICE_NAME! of OceanStor BCManager Agent is not exist.
        call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent is not exist."
        timeout /T 3 /NOBREAK >nul
        exit /b 1
    )
    
    for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "STOPPED"') do (set SERVICE_CHECK=%%i)
    if not "!SERVICE_CHECK!" == "" (
    
        echo The service !SERVICE_NAME! of OceanStor BCManager Agent was already stopped.
        call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent was already stopped."
        
    ) else (
    
        sc stop !SERVICE_NAME! >>%LOGFILE_PATH% 2>&1
        
        :waitstop
            timeout /T 3 /NOBREAK >nul
            
            set SERVICE_CHECK=
            for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "STOPPED"') do (set SERVICE_CHECK=%%i)
            
            if not "!SERVICE_CHECK!" == "" (
            
                echo The service !SERVICE_NAME! of OceanStor BCManager Agent was stopped now.
                call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent was stopped now."
                
            ) else (
                rem retry 5 times 
                if !RETRY_COUNT! LEQ 5 (
                    set /a RETRY_COUNT+=1
                    call :Log "Wait the service !SERVICE_NAME! status is stopped, and retry !RETRY_COUNT! times after 3 sec."
                    goto :waitstop
                )
                
                echo The service !SERVICE_NAME! of OceanStor BCManager Agent fails to be stopped.
                call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent fails to be stopped  after retry 5 times, exit 1"
                timeout /T 3 /NOBREAK >nul
                exit /b 1
                
            )
    )

    exit /b 0
    
:checkservice
    call :Log "Check Service %~1."
    set CHECK=
    for /f "delims=" %%i in ('2^>nul sc query %~1 ^| findstr /i "%~1"') do (set CHECK=%%i)
    if "!CHECK!" == "" (
        rem Service %~1 is not exist.
        exit /b 1
    )
    
    rem Service %~1 is exist
    exit /b 0

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILE_PATH%
    )
    
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILE_PATH%
    
    goto :EOF
    
:end
    endlocal