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

rem @dest:   application agent for querying oracle tablespace information
rem @date:   2020-07-06
rem @authr:  
rem @modify: 

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERROR_DBUSERPWD_WRONG=10
set /a ERROR_RECOVER_INSTANCE_NOSTART=11
set /a ERROR_INSUFFICIENT_WRONG=15

set /a ERROR_ASM_DBUSERPWD_WRONG=21
set /a ERROR_ASM_INSUFFICIENT_WRONG=22
set /a ERROR_ASM_RECOVER_INSTANCE_NOSTART=23
set /a ERROR_ORACLE_NOARCHIVE_MODE=24
set /a ERROR_ORACLE_OVER_ARCHIVE_USING=25
set /a ERROR_ASM_DISKGROUP_ALREADYMOUNT=26
set /a ERROR_ASM_DISKGROUP_NOTMOUNT=27
set /a ERROR_APPLICATION_OVER_MAX_LINK=28
set /a ERROR_DB_ALREADY_INBACKUP=29
set /a ERROR_DB_INHOT_BACKUP=30
set /a ERROR_DB_ALREADYRUNNING=31
set /a ERROR_DB_ALREADYMOUNT=32
set /a ERROR_DB_ALREADYOPEN=33
set /a ERROR_DB_ARCHIVEERROR=34

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"

set CMD_GETVERSION=getversion
set CMD_EXECSQL=execsql
set CMD_GETVALUE=getvalue

set DB_LOGIN=
set ASM_LOGIN=

set LOGFILE=oraclequerytablespace.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

rem check instance status
set QuerySQL="%AGENT_TMP_PATH%Query%PID%.sql"
set QuerySQLRST="%AGENT_TMP_PATH%QueryRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
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
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
call :UperTOLow !DBUSERL! DBUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)

call :SetDBAuth

rem get current process id
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
            more %CURRENTCMDLineRST% | findstr /c:"oraclequerytablespace.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclequerytablespace.bat %AGENT_ROOT% %PID%"
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

echo DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%
call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;CURRENTPID=!CURRENTPID!;"

set DBROLE=
if "%DBUSER%"=="sys" (
    set DBROLE=as sysdba
)
rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION
set PREDBVERSION=%ORA_VERSION:~0,4%

call :QueryTableSpaceInfo
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

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    set %~2=!CONVERTSTR!
goto :EOF

:QueryTableSpaceInfo
    if "!PREDBVERSION!" LSS "12" (
        echo set linesize 600 > %QuerySQL%
        echo set heading off >> %QuerySQL%
        echo set newpage none >> %QuerySQL%
        echo set feedback off >> %QuerySQL%
        echo set pagesize 0 >> %QuerySQL%
        echo col TSNAME for a30 >> %QuerySQL%
        echo col FILENAME for a113 >> %QuerySQL%
        echo col CONNAME for a30 >> %QuerySQL%
        echo select 'NOCDB' CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v$datafile A, v$tablespace C where A.TS#=C.TS# order by TSNAME; >>%QuerySQL%
    ) else (
        echo set linesize 600 > %QuerySQL%
        echo set heading off >> %QuerySQL%
        echo set newpage none >> %QuerySQL%
        echo set feedback off >> %QuerySQL%
        echo set pagesize 0 >> %QuerySQL%
        echo col TSNAME for a30 >> %QuerySQL%
        echo col FILENAME for a113 >> %QuerySQL%
        echo col CONNAME for a30 >> %QuerySQL%
        echo select 'NOCDB' CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v$datafile A, v$tablespace C where A.CON_ID=0 and A.TS#=C.TS# order by CONNAME, TSNAME; >> %QuerySQL%
        echo select 'CDB' CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v$datafile A, v$tablespace C where A.CON_ID=1 and A.TS#=C.TS# order by CONNAME, TSNAME; >> %QuerySQL%
        echo select B.NAME CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v$datafile A, v$pdbs B, v$tablespace C where A.TS#=C.TS# and A.CON_ID=B.CON_ID order by CONNAME, TSNAME; >> %QuerySQL%
    )

    call :Log "Exec sql to get database's tablespace."
    call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %QuerySQL% %QuerySQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Execute Script to query DB tablespace list failed."
        call :DeleteFile %QuerySQL%
        call :DeleteFile %QuerySQLRST%
        exit !RetCode!
    ) else (
        call :DeleteFile %RSTFILE%
        for /f "tokens=1-4 delims=*" %%a in ('type %QuerySQLRST%') do (
            set line=
            call :TrimString "%%a" str
            if not "!str!" == "" (
                set line=!line!!str!;
            )
            call :TrimString "%%b" str
            if not "!str!" == "" (
                set line=!line!!str!;
            )
            call :TrimString "%%c" str
            if not "!str!" == "" (
                set line=!line!!str!;
            )
            call :TrimString "%%d" str
            if not "!str!" == "" (
                set line=!line!!str!
            )
            
            echo !line!>> %RSTFILE%
        )

        call :DeleteFile %QuerySQL%
        call :DeleteFile %QuerySQLRST%
        echo Query %DBNAME% tablespace infor successful.
        call :Log "Query %DBNAME% tablespace infor successful."
    )
goto :eof

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

:TrimString
    set trimString=%~1
    call :LeftTrim
    call :RightTrim
    set %~2=!trimString!
goto :EOF

:LeftTrim
if "%trimString:~0,1%"==" " (
    set "trimString=%trimString:~1%"
    goto LeftTrim
)
if "%trimString:~0,1%"=="	" (
    set "trimString=%trimString:~1%"
    goto LeftTrim
)
goto :eof

:RightTrim
if "%trimString:~-1%"==" " (
    set "trimString=%trimString:~0,-1%"
    goto RightTrim
)
if "%trimString:~-1%"=="	" (
    set "trimString=%trimString:~0,-1%"
    goto RightTrim
)
goto :eof

endlocal
