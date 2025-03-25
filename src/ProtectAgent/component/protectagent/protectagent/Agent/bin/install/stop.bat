@echo off
setlocal EnableDelayedExpansion

set CMD_PAUSE=pause

set DATA_BACKUP_AGENT_HOME_VAR=
for /f "tokens=2 delims==" %%a in ('wmic environment where "name='DATA_BACKUP_AGENT_HOME' and username='<system>'" get VariableValue /value') do (
    if not "%%a" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%%a
    )
)
if "%DATA_BACKUP_AGENT_HOME_VAR%" == "" (
    set WIN_SYSTEM_DISK=%WINDIR:~0,1%
    set DATA_BACKUP_AGENT_HOME_VAR=C:
    if not "%WIN_SYSTEM_DISK%" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%WIN_SYSTEM_DISK%:
    )
)
set AGENT_BIN_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\ProtectClient\ProtectClient-E\bin
rem ---------------------------------------------------------------
rem               the big package stop script
rem ---------------------------------------------------------------

net.exe session 1>NUL 2>NUL || (
    echo Please run the script as an administrator.
    %CMD_PAUSE%
    exit /b 1
)

:SelectStop
    if exist "%AGENT_BIN_PATH%" (
        call %AGENT_BIN_PATH%\agent_stop.bat
        %CMD_PAUSE%
        exit /b 0
    ) else (
        call :gotoexit
    )

    goto :EOF

:gotoexit
    echo Not install any client,the stop will be stopped.
    %CMD_PAUSE%
    exit /b 1

endlocal