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

set QUOTE=;
set ORACLESERVICEPRE=ORACLESERVICE
set ORACLEVSSWRITERPRE=ORACLEVSSWRITER

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set CMD_GETVERSION=getversion
set CMD_GETORAPATH=getoraclepath
set CMD_GETGRIDPATH=getgridpath
set LOGFILE=oracleinfo.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set /a ERROR_SCRIPT_EXEC_FAILED=5

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"

call :DeleteFile %RSTFILE%
rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION

rem ************************get oracle path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETORAPATH% %PID% %LOGFILE% ORACLEBASEPATH ORACLEHOMEPATH

if "!ORACLEBASEPATH!" == "" (
    call :Log "Get Oracle base path failed." 
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

if "!ORACLEHOMEPATH!" == "" (
    call :Log "Get Oracle home path failed." 
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

set DBISCLUSTER=0
rem ************************get grid path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETGRIDPATH% %PID% %LOGFILE% !ORA_VERSION! GRIDHOMEPATH
if "!GRIDHOMEPATH!" == "" (
    call :Log "Get grid home path failed." 
)
rem check if iscluster
if not "!GRIDHOMEPATH!" == "" (
    if exist "!GRIDHOMEPATH!\bin\crsctl.exe" (
        "!GRIDHOMEPATH!\bin\crsctl.exe" check help | findstr /c:"crsctl check crs">nul
        if !errorlevel! EQU 0 (
            set DBISCLUSTER=1
        )
    )
)
call :Log "Check cluster [DBISCLUSTER=!DBISCLUSTER!]."

rem check ASM instance
set STATUSTMP=1
set authType=0
for /f "tokens=2 delims=:" %%i in ('sc query OracleASMService+ASM ^| findstr /i "STATE"') do (
    for /f "tokens=1 delims= " %%j in ("%%i") do (
        if %%j EQU 4 (
            set /a STATUSTMP=0
            call :CheckInstAuth +ASM authType
        )
    )
    call :Log "+ASM;!authType!;!DBISCLUSTER!"
    echo +ASM;!authType!;!DBISCLUSTER!>> %RSTFILE%
)
exit 0

:CheckInstAuth
    set INSTNAME=%1
    set FSTCHAR=!INSTNAME:~0,1!
    set CheckAuthType="%AGENT_TMP_PATH%CheckAuthType%PID%.sql"
    set CheckAuthTypeRST="%AGENT_TMP_PATH%CheckAuthTypeRST%PID%.txt"

    echo exit; > %CheckAuthType%
    if "!FSTCHAR!" == "+" (
        set DB_LOGIN=/ as sysdba
        call %COMMONFUNC% "%AGENT_ROOT%" execsql %PID% %LOGFILE% %CheckAuthType% %CheckAuthTypeRST% %INSTNAME% "!DB_LOGIN!" 30 RetCode
    ) else (
        set ASM_LOGIN=/ as sysasm
        call %COMMONFUNC% "%AGENT_ROOT%" execasmsql %PID% %LOGFILE% %CheckAuthType% %CheckAuthTypeRST% %INSTNAME% "!ASM_LOGIN!" 30 RetCode
    )
    call :DeleteFile %CheckAuthType%
    call :DeleteFile %CheckAuthTypeRST%
    if "!RetCode!" NEQ "0" (
        call :Log "Check instance name %INSTNAME% auth type failed."
        set %~2=0
    ) else (
        set %~2=1
    )
goto :EOF

rem ************************************************************************
rem function name: DeleteFile
rem aim:           Delete file function
rem input:         the deleted file
rem output:        
rem ************************************************************************
:DeleteFile
    set FileName=%~1
    if exist "%FileName%" (del /f /q "%FileName%")
    
goto :EOF

rem ************************************************************************
rem function name: Log
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
goto :EOF

endlocal
