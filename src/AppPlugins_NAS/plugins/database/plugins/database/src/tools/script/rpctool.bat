@echo off
rem ########################################################################
rem # DB RPC Tool
rem ########################################################################

set CUR_PATH=%~dp0
echo %CUR_PATH%
set INTERFACE_NAME=%~1
echo %INTERFACE_NAME%
set INPUT_FILE_NAME=%~2
echo %INPUT_FILE_NAME%
set OUTPUT_FILE_NAME=%~3
echo %INPUT_FILE_NAME%
set LOG_PATH=%CUR_PATH%..\..\..\ProtectClient-E\log\Plugins\GeneralDBPlugin
echo %LOG_PATH%
set THRIFT_SERVER_PORT_FILE_PATH=%CUR_PATH%..\..\..\ProtectClient-E\tmp\thriftserverport
echo %THRIFT_SERVER_PORT_FILE_PATH%
set /p THRIFT_SERVER_PORT=<%THRIFT_SERVER_PORT_FILE_PATH%
echo %THRIFT_SERVER_PORT%

%CUR_PATH%rpctool.exe %INTERFACE_NAME% %INPUT_FILE_NAME% %OUTPUT_FILE_NAME% %LOG_PATH% %THRIFT_SERVER_PORT%
if %errorlevel% equ 0 (
    echo Call dbrpctool success.
) else (
    echo Call dbrpctool failed.
    exit /b 1
)
exit /b 0