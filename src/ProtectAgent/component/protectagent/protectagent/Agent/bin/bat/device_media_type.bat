@echo off
setlocal EnableDelayedExpansion

set AGENT_ROOT_PATH=%1
set RANDOM_ID=%2
cd /d %~dp0

set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\\bin\\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\\log\\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\\tmp\\
set NEED_LOG=1
set LOGFILE_PATH=%AGENT_LOG_PATH%device_media_type.log
set RESULT_FILE=%AGENT_TMP_PATH%result_tmp%RANDOM_ID%

set ERR_CODE=1
set ERR_SCRIPT_EXEC_FAILED=5
set ERR_SCRIPT_NOT_EXIST=255
set ERR_RESULT_FILE_NOT_EXIST=6
set ERR_FILE_IS_EMPTY=8

rem Get powershell path
call :log "Begin to get powershell path."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1\PowerShellEngine" /v "ApplicationBase"') do set "POWERSHELL_PATH=%%k"
if "!POWERSHELL_PATH!"=="" (
    call :log "Get powershell path faild."
    set ERR_CODE=%ERR_FILE_IS_EMPTY%
    goto error
)

call :log "Get powershell path succ, powershell path is : %POWERSHELL_PATH%."

rem Excute powershell script to get device media type
if not exist %AGENT_BIN_PATH%device_media_type.ps1 (
    set ERR_CODE=%ERR_SCRIPT_NOT_EXIST%
    call :log "Script device_media_type.ps1 is not exist, exit code %ERR_CODE%"
    goto error
)

call :log "Begin to execute device_media_type.ps1 powershell, result file result_tmp%RANDOM_ID%."
"%POWERSHELL_PATH%\powershell.exe" -command ".\device_media_type.ps1 '%RESULT_FILE%'"; exit $LASTEXITCODE

if errorlevel 1 (
	if exist %RESULT_FILE% (
       del /f /q %RESULT_FILE%
    )
    call :log "Execute powershell[device_media_type.ps1] failed, errorlevel is [!errorlevel!]."
    set ERR_CODE=%ERR_SCRIPT_EXEC_FAILED%
    goto error
) else (
    if not exist %RESULT_FILE% (
        set ERR_CODE=%ERR_RESULT_FILE_NOT_EXIST%
        call :log "Execute powershell[device_media_type.ps1] failed, result file is not exist, exit code !ERR_CODE!"
        goto error
    )
    call :log "Execute powershell[device_media_type.ps1] succ. errorlevel is [!errorlevel!]"
    goto end
)

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILE_PATH%
    )
    call %AGENT_BIN_PATH%\agent_func.bat %LOGFILE_PATH%
    goto :EOF

:error
    call :log "Get device media type failed."
    exit /b %ERR_CODE%

:end
    endlocal

