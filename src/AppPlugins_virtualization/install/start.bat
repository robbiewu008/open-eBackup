@echo off
rem ########################################################################
rem # FilePlugin start.bat
rem ########################################################################

set LOG_PATH=%~1
set PORT_BEGIN=%~2
set PORT_END=%~3
set AGENT_IP=%4
set AGENT_PORT=%5

set CurBatPath=%~dp0
set START_LOG=%LOG_PATH%start.log

setlocal EnableDelayedExpansion

start %CurBatPath%bin\VirtualizationPlugin.exe %LOG_PATH% %PORT_BEGIN% %PORT_END% %AGENT_IP% %AGENT_PORT%

endlocal
exit 0