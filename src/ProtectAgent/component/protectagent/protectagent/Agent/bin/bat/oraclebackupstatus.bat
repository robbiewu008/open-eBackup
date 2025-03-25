@echo off
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
rem @date:   2020-04-30
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

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set EXECSTATUS="%AGENT_TMP_PATH%exec_status%PID%"
set LOGFILE=oraclebackupstatus.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
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

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "TaskQueryType" TASKTYPE

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetDBAuth
set ORACLEDBSERVICE=OracleService!DBINSTANCE!

rem get current process id for excute sql timeout and log
call wmic process where name="cmd.exe" get processid > %CURRENTPIDRST%
set /a NUM=0
for /f %%a in ('type %CURRENTPIDRST%') do (
    if !NUM! NEQ 0 (
        set processID=%%a
        call wmic process where processid="!processID!" get Commandline > %CURRENTCMDLineRST%
        set AGENT_ROOT_TMP=%AGENT_ROOT: =%
        set BLACKFLAG=1
        if "!AGENT_ROOT_TMP!" == "!AGENT_ROOT!" (
            set BLACKFLAG=0
        )
        if "!BLACKFLAG!" == "1" (
            more %CURRENTCMDLineRST% | findstr /c:"oraclecheckdbstatus.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclecheckdbstatus.bat %AGENT_ROOT% %PID%"
        )
        if !errorlevel! EQU 0 (
            set /a CURRENTPID=%%a
        )
    )
    set /a NUM=!NUM!+1
)

call :DeleteFile %CURRENTPIDRST%
call :DeleteFile %CURRENTCMDLineRST%
if !CURRENTPID! EQU 0 (
    call :Log "CURRENTPID equal 0, the timeout is will failed."
)
set monitorPIDs="%AGENT_TMP_PATH%ProcMonitorLists!CURRENTPID!.txt"

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;TASKTYPE=!TASKTYPE!"
call :DeleteFile %RSTFILE%

rem begin to get backup status
echo set linesize 300; > %CHECKINSTANCESTATUS%
echo set pagesize 0 >> %CHECKINSTANCESTATUS%
echo set heading off >> %CHECKINSTANCESTATUS%
echo set feedback off >> %CHECKINSTANCESTATUS%
echo select p.SOFAR, '*', p.TOTALWORK, '*', p.progress from(SELECT SOFAR, TOTALWORK, ROUND(SOFAR/TOTALWORK*100,2) progress FROM V$SESSION_LONGOPS where opname like 'RMAN%%' and totalwork^<^>0 and SOFAR ^<^> TOTALWORK order by start_time desc)p where rownum = 1; >> %CHECKINSTANCESTATUS%
echo exit >> %CHECKINSTANCESTATUS%

call :DeleteFile %CHECKINSTANCESTATUSRST%

call :Log "Exec sql to get status of instance %DBINSTANCE%."
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
if "!RetCode!" NEQ "0" (
    set ERR_CODE=!RetCode!
    goto :error
)

set SOFAR=
set TOTALWORK=
set PROGRESS=
set SPEED=
for /f "tokens=1-3 delims=*	 " %%a in ('type %CHECKINSTANCESTATUSRST%') do (
    set SOFAR=%%a
    set TOTALWORK=%%b
    set PROGRESS=%%c
)

rem must save into var, against can not save into file when operation executed in body of if
set SQLInfo=select p.* from(SELECT TRIM (output_bytes_per_sec_display) backupSpeed FROM v$rman_backup_job_details where start_time ^> TRUNC (SYSDATE) - 12 and status='RUNNING' order by start_time desc)p where rownum = 1;
rem restore tasktype is null, don't query backup speed
if "!TASKTYPE!" == "0" (
    echo set linesize 50 > %CHECKINSTANCESTATUS%
    echo set pagesize 0 >> %CHECKINSTANCESTATUS%
    echo set heading off >> %CHECKINSTANCESTATUS%
    echo set feedback off >> %CHECKINSTANCESTATUS%
    echo col backupSpeed for a12 >> %CHECKINSTANCESTATUS%

    echo !SQLInfo! >> %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

    call :DeleteFile %CHECKINSTANCESTATUSRST%
    call :Log "Exec sql to get oracle backup speed %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        set ERR_CODE=!RetCode!
        goto :error
    )
    for /f %%a in ('type %CHECKINSTANCESTATUSRST%') do (
        set SPEED=%%a
    )
)

echo !SPEED!;!SOFAR!;!TOTALWORK!;!PROGRESS! > !RSTFILE!
call :Log "Finish getting database status, !SPEED!;!SOFAR!;!TOTALWORK!;!PROGRESS!"
set ERR_CODE=0
goto :error

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

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )

    set %~2=!CONVERTSTR!
goto :EOF

:SetDBAuth
    if !AUTHMODE!==1 (
        set DB_LOGIN=/ as sysdba
    )
    if !AUTHMODE!==0 (
        if "%DBUSER%"=="sys" (
            set DB_LOGIN=%DBUSER%/"%DBUSERPWD%" as sysdba
        ) else (
            set DB_LOGIN=%DBUSER%/"%DBUSERPWD%"
        )
    )
goto :eof

:error
    call :DeleteFile %CHECKINSTANCESTATUS%
    call :DeleteFile %CHECKINSTANCESTATUSRST%
    exit %ERR_CODE%

endlocal
