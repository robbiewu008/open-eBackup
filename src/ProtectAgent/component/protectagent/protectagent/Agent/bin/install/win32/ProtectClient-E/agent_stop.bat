@echo off
setlocal EnableDelayedExpansion
::The script is used to start Agent Service.

set PARAM1=%~1

set CURRENT=%~dp0
set AGENT_ROOT=%CURRENT%..\
set AGENT_PLUGIN_PATH=%AGENT_ROOT%..\Plugins
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%log\
set LOGFILE_PATH="%AGENT_LOG_PATH%agent_stop.log"
set NEED_LOG=1
set IS_UPGRADE=0

echo Begin stop DataBackup ProtectAgent services...
echo.

if NOT "%PARAM1%" EQU "" (
    if "%PARAM1%" EQU "upgrade" (
        set IS_UPGRADE=1
    )
)

if "%PARAM1%" == "rdmonitor" (
    rem stop dataturbo service
    call :stoptservice rdmonitor
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

if "%PARAM1%" == "rdagent" (
    rem stop dataturbo service
    call :stoptservice rdagent
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

if "%PARAM1%" == "rdnginx" (
    rem stop dataturbo service
    call :stoptservice rdnginx
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

if "%PARAM1%" == "dataturbo" (
    rem stop dataturbo service
    call :stoptservice dataturbo
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

rem stop rdmonitor service
call :stoptservice rdmonitor
if not "!errorlevel!" == "0" (exit /b 1)

rem stop rdagent service
call :stoptservice rdagent
if not "!errorlevel!" == "0" (exit /b 1)

rem stop rdnginx service
call :stoptservice rdnginx
if not "!errorlevel!" == "0" (exit /b 1)

rem stop Plugins
for /f "delims=" %%a in ('dir !AGENT_PLUGIN_PATH! /b /a:d') do (
    set PLUGIN_NAME=%%a
	if exist "!AGENT_PLUGIN_PATH!\!PLUGIN_NAME!\stop.bat" (
		echo Stop plugin !PLUGIN_NAME!.
		call :Log "Stop plugin !PLUGIN_NAME!."
		call "!AGENT_PLUGIN_PATH!\!PLUGIN_NAME!\stop.bat"
        if not "!errorlevel!" == "0" (
            echo Stop Plugins failed.
            call :Log "Stop Plugins failed."
            exit /b 1
        )
	)
)
echo.
echo DataBackup ProtectAgent services has been successfully stopped.

call :Winsleep 3
call :Log "DataBackup ProtectAgent services has been successfully stopped."

endlocal
exit /b 0

:stoptservice
    set SERVICE_CHECK=
    set SERVICE_NAME=%~1
    set RETRY_COUNT=1
    set SC_RETRY_COUNT=1
    
    call :checkservice !SERVICE_NAME!
    if %errorlevel% EQU 2 (
        call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent is not exist, it is a normal situation during the upgrade process."
        exit /b 0
    )
    if not "!errorlevel!" == "0" (
        echo The service !SERVICE_NAME! of DataBackup ProtectAgent is not exist.
        call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent is not exist."
        call :Winsleep 3
        exit /b 1
    )
    
    for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "STOPPED"') do (set SERVICE_CHECK=%%i)
    if not "!SERVICE_CHECK!" == "" (
    
        echo The service !SERVICE_NAME! of DataBackup ProtectAgent was already stopped.
        call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent was already stopped."
        
    ) else (
    
        :stopagentservice
            sc stop !SERVICE_NAME! >>%LOGFILE_PATH% 2>&1
            
            :waitstop
                call :Winsleep 3
                
                set SERVICE_CHECK=
                for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "STOPPED"') do (set SERVICE_CHECK=%%i)
                
                if not "!SERVICE_CHECK!" == "" (
                
                    echo The service !SERVICE_NAME! of DataBackup ProtectAgent was stopped now.
                    call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent was stopped now."
                    
                ) else (
                    rem retry 5 times 
                    if !RETRY_COUNT! LEQ 5 (
                        set /a RETRY_COUNT+=1
                        call :Log "Wait the service !SERVICE_NAME! status is not stopped, and retry !RETRY_COUNT! times after 3 sec."
                        goto :waitstop
                    )

                    if !SC_RETRY_COUNT! LEQ 3 (
                        set /a SC_RETRY_COUNT+=1
                        call :Log "stop service !SERVICE_NAME! faild, and retry !SC_RETRY_COUNT! times after 15 sec."
                        set RETRY_COUNT=1
                        goto :stopagentservice
                    )
                    
                    echo The service !SERVICE_NAME! of DataBackup ProtectAgent fails to be stopped.
                    call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent fails to be stopped  after retry 3 times, exit 1"
                    call :Winsleep 3
                    exit /b 1
                    
                )
    )

    exit /b 0
    
:checkservice
    call :Log "Check Service %~1."
    set CHECK=
    for /f "delims=" %%i in ('2^>nul sc query %~1 ^| findstr /i "%~1"') do (set CHECK=%%i)
    if "!CHECK!" == "" (
        if %IS_UPGRADE% EQU 1 (
            rem This is a normal situation during the upgrade process
            exit /b 2
        )
        rem Service %~1 is not exist.
        exit /b 1
    )
    
    rem Service %~1 is exist
    exit /b 0

:Winsleep
    ping 127.0.0.1 -n %~1 > nul
    exit /b 0
	
:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILE_PATH%
    )
    
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILE_PATH%
    
    goto :EOF
