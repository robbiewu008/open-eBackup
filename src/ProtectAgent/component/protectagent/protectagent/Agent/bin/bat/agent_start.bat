@echo off
setlocal EnableDelayedExpansion
::The script is used to start Agent Service.

set CURRENT=%~dp0
set AGENT_ROOT=%CURRENT%..\
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%log\
set LOGFILE_PATH=%AGENT_LOG_PATH%agent_start.log
set WORKING_USER=rdadmin
set NGINX_SERVICE_NAME=rdnginx
rem WORKING_USER_PWD is used for changing user. It can be any value but not empty.
set WORKING_USER_PWD=pwd
set DS_REPAIR_MODE=DsRepair
set NEED_LOG=1

echo Begin start OceanStor BCManager Agent services...
echo.

rem Check windows satrt status. We need to support directory repair mode.
call :ChecksystemStartStatus
if not "!errorlevel!" == "0" (exit /b 1)

rem start rdmonitor service
call :startservice rdnginx
if not "!errorlevel!" == "0" (exit /b 1)

rem start rdagent service
call :startservice rdagent
if not "!errorlevel!" == "0" (exit /b 1)

rem start rdnginx service
call :startservice rdmonitor
if not "!errorlevel!" == "0" (exit /b 1)

echo.
echo OceanStor BCManager Agent services was started successfully.
call :Log "OceanStor BCManager Agent services was started successfully."

timeout /T 3 /NOBREAK >nul
goto :end

:startservice
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
    
    for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
    if not "!SERVICE_CHECK!" == "" (
    
        echo The service !SERVICE_NAME! of OceanStor BCManager Agent is already exist.
        call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent is already exist."
        
    ) else (
    
        sc start !SERVICE_NAME! >>"%LOGFILE_PATH%" 2>&1
        
        :waitstart
            timeout /T 3 /NOBREAK >nul
            
            set SERVICE_CHECK=
            for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
            
            if not "!SERVICE_CHECK!" == "" (
            
                echo The service !SERVICE_NAME! of OceanStor BCManager Agent is running now.
                call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent is running now."
                
            ) else (
                rem retry 5 times 
                if !RETRY_COUNT! LEQ 5 (
                    set /a RETRY_COUNT+=1
                    call :Log "Wait the service !SERVICE_NAME! status is stopped, and retry !RETRY_COUNT! times after 3 sec."
                    goto :waitstart
                )
                
                echo The service !SERVICE_NAME! of OceanStor BCManager Agent was started failed.
                call :Log "The service !SERVICE_NAME! of OceanStor BCManager Agent was started failed after retry 5 times, exit 1."
                timeout /T 3 /NOBREAK >nul
                exit /b 1
                
            )
    )

    exit /b 0

:ChecksystemStartStatus
    set STATUS=
    for /f "tokens=2 delims=: " %%a in ('bcdedit') do (
        if "%%a"=="%DS_REPAIR_MODE%" (
            set STATUS=%DS_REPAIR_MODE%
        )
    )

    set USER_NAME=
    for /f "tokens=2 delims='='" %%a in ('wmic service where "name='%NGINX_SERVICE_NAME%'" get startname /value') do (
        set USER_NAME=%%a
    )
    set USER_NAME=%USER_NAME:.\=%

    if not "!STATUS!" == "%DS_REPAIR_MODE%" (
        if not "!USER_NAME!" == "%WORKING_USER%" (
            echo "Normal boot start!"
            call :Log "Normal boot start!"
            call "%AGENT_BIN_PATH%\winservice.exe" rdagent changeUser !WORKING_USER! !WORKING_USER_PWD!
            call "%AGENT_BIN_PATH%\winservice.exe" nginx changeUser !WORKING_USER! !WORKING_USER_PWD!
        )
    ) else (
        if "!USER_NAME!" == "%WORKING_USER%" (
            echo "Directory repair mode start!"
            call :Log "Directory repair mode start!"
            call "%AGENT_BIN_PATH%\winservice.exe" nginx changeUser
            call "%AGENT_BIN_PATH%\winservice.exe" rdagent changeUser
        )
    )

    if not "!errorlevel!" == "0" (
        call :Log "Change user of service failed!"
        exit /b 1
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
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
    )
    
    call "%AGENT_BIN_PATH%\agent_func.bat" "%LOGFILE_PATH%"
    
    goto :EOF
    
:end
    endlocal