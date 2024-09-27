@echo off
rem ########################################################################
rem # DBPlugin start.bat
rem ########################################################################

set LOG_PATH=%~1
set PORT_BEGIN=%~2
set PORT_END=%~3
set AGENT_IP=%4
set AGENT_PORT=%5

set PYTHON_PATH=%DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\install\Python310
set Path=%PYTHON_PATH%\;%PYTHON_PATH%\Scripts\;%Path%
set WORKON_HOME=%DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\install\virtualenvs
set START_LOG=%CurBatPath%start.log

setlocal EnableDelayedExpansion

call %PYTHON_PATH%\Scripts\workon.bat plugin_env
if %errorlevel% NEQ 0 (
    echo Enter virtual env failed! >> %START_LOG%
    exit /b 1
)
echo Enter virtual env success! >> %START_LOG%

start %DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\bin\GeneralDBPlugin.exe %LOG_PATH% %PORT_BEGIN% %PORT_END% %AGENT_IP% %AGENT_PORT%

endlocal
exit /b 0