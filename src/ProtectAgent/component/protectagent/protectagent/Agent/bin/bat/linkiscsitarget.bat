rem @echo off
:: 
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
::
rem @dest:   application agent for oracle
rem @date:   2020-04-21
rem @authr:  
rem @modify:

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERR_FILE_IS_EMPTY=8

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"
set CMD_GETVALUE=getvalue
set CMD_EXECSQL_SILE=execsqls

set LOGFILE=linkiscsitarget.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
rem #############################################################

set INPUTINFO=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTINFO=%%a
    )
)
call :DeleteFile %PARAM_FILE%

if "!INPUTINFO!" == "" (
    call :Log "INPUTINFO is null."
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :error
)

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "targetIp" targetIp
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "targetPort" targetPort
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "chapName" chapName
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "chapPwd" chapPwd
call :Log "targetIp=%targetIp%;targetPort=%targetPort%;chapName=%chapName%"

call :Log "Begin to link iscsi target."
rem Check the iscsi service status
for /f "tokens=2 delims=:" %%i in ('sc query msiscsi ^| findstr /i "STATE"') do (
    set SERVICESTATUS=%%i
    for /f "tokens=1 delims= " %%j in ("!SERVICESTATUS!") do (
        set /a oraStatus=%%j
        rem Check Service is Start
        if !oraStatus! NEQ 4 (
            call :Log "The msiscsi Service isn't running."
            sc config msiscsi start= auto >> %LOGFILEPATH%
            net start msiscsi >> %LOGFILEPATH%
        )
    )
)

rem get target iqn name
iscsicli ListTargets | findstr !targetIp! >> %LOGFILEPATH%
if !errorlevel! NEQ 0 (
    rem start link target
    iscsicli AddTargetPortal !targetIp! !targetPort! >> %LOGFILEPATH%
    if !errorlevel! NEQ 0 (
        call :Log "discovery iscsi target !targetIp! failed."
        exit 1
    ) else (
        call :Log "Add target !targetIp! successfully."
    )
)

rem check connection status
iscsicli SessionList | findstr !targetIp! >> %LOGFILEPATH%
if !errorlevel! NEQ 0 (
    rem get target iqn name
    for /f "tokens=1 delims= " %%a in ('iscsicli ListTargets ^| findstr !targetIp!') do (
        set iqnName=%%a
        if "!chapName!" == "" (
            iscsicli QLoginTarget !iqnName!
        ) else (
            iscsicli QLoginTarget !iqnName!
        )

        if !errorlevel! NEQ 0 (
            call :Log "link iscsi target failed."
            exit 1
        ) else (
            call :Log "link iscsi target successfully."
        )
    )
) else (
    call :Log "iscsi target !targetIp! is already linked."
)

call :Log "Finish linking iscsi target."
exit 0

rem Print log function, controled by "NEEDLOGFLG".
:Log
    echo %date:~0,10% %time:~0,8% [%username%] [!CURRENTPID!] %~1 >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
    goto :EOF

rem Delete file function, it can delete many files.
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
    goto :EOF

endlocal
