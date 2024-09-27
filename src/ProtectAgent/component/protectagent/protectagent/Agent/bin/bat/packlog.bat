@echo off

setlocal EnableDelayedExpansion
set AGENT_ROOT_PATH=%~1
set ID=%~2

set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\log\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\tmp\
set NGINX_LOG_PATH=%AGENT_BIN_PATH%\nginx\logs\
set ZIP_TOOL=%AGENT_BIN_PATH%\7z.exe
set INPUT_TMP_FILE=%AGENT_TMP_PATH%input_tmp%ID%
set ARGFILE="%AGENT_TMP_PATH%ARG%ID%.txt"
set LOG_FILE_NAME="%AGENT_LOG_PATH%packlog.log"
set CMD_GETVALUE=getvalue

call :Log "Begin to package log."
if not exist "%INPUT_TMP_FILE%" (
    call :Log "input_tmp%ID% is not exists."
    endlocal
    exit /b 1
)

for /f "delims=" %%a in ('type %INPUT_TMP_FILE%') do (
    if not "%%a" == "" (
        echo %%a | findstr "^operation" > nul
        if !errorlevel! equ 0 (
            call :GetValue "%%a" "operation"
            set OPERTYPE=!ArgValue!
        ) else (
            call :GetValue "%%a" "logPackName"
            set LOG_FOLDER=!ArgValue!
        )
    )
)
if exist %INPUT_TMP_FILE% (del /f /q %INPUT_TMP_FILE%)

call :Log "Operation:%OPERTYPE%"
call :Log "Log name:%LOG_FOLDER%"

SET PACKAGE_LOG=%AGENT_TMP_PATH%%LOG_FOLDER%

if "%OPERTYPE%"=="collect" (
    if exist %INPUT_TMP_FILE% (del /f /q "%INPUT_TMP_FILE%")
    del /f /q "%AGENT_TMP_PATH%\AGENTLOG_*"

    mkdir "%PACKAGE_LOG%"
    mkdir "%PACKAGE_LOG%\nginx_log"
    mkdir "%PACKAGE_LOG%\agent_log"
    mkdir "%PACKAGE_LOG%\plugin_log"

    if exist "%NGINX_LOG_PATH%"    copy /y  "%NGINX_LOG_PATH%"*.log*   "%PACKAGE_LOG%\nginx_log\"  >Nul
    if exist "%AGENT_LOG_PATH%"    copy /y  "%AGENT_LOG_PATH%"*.log*   "%PACKAGE_LOG%\agent_log\"  >Nul
    if exist "%AGENT_LOG_PATH%" (
        if exist "%AGENT_LOG_PATH%"Plugins (
            xcopy /e/h/v/q/y  "%AGENT_LOG_PATH%"Plugins\*   "%PACKAGE_LOG%\plugin_log\"  >Nul
        )
    )

    call :Log "Compress agent log."
    "%ZIP_TOOL%"  a -tzip "%PACKAGE_LOG%.zip"  "%PACKAGE_LOG%\*" -mx=9 >Nul

    rmdir  /s/q   "%PACKAGE_LOG%"
    call :Log "Finish packaging log."
    endlocal
    exit /b 0
) else if "%OPERTYPE%"=="clean" (
    del /f /q "%PACKAGE_LOG%.zip"
    endlocal
    exit /b 0
) else (
    call :Log "Invalid operation type."
    endlocal
    exit /b 1
)

:GetValue
    if exist %ARGFILE% (del /f /q %ARGFILE%)
    set ArgInput=%~1
    set ArgName=%~2
    set ArgValue=--
    for /l %%a in (1 1 20) do call :Separate %%a

    for /f "tokens=1,2 delims==" %%i in ('type %ARGFILE%') do (
        if %%i==%ArgName% (
            set ArgValue=%%j
        )
    )
    if exist %ARGFILE% (del /f /q %ARGFILE%)
goto :EOF

:Separate
    for /f "tokens=%1 delims=;" %%i in ("%ArgInput%") do (
        echo %%i>> %ARGFILE%
    )
goto :EOF

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