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

set EXECSQL=CheckInstanceStatus%PID%.sql
set EXECSQLRST=CheckInstanceStatusRST%PID%.txt

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclenativeexpire.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
rem #############################################################

set INPUTINFO=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTINFO=%%a
    )
)
rem call :DeleteFile %PARAM_FILE%

if "!INPUTINFO!" == "" (
    call :Log "INPUTINFO is null."
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :error
)

call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAMETMP
if "!DBNAMETMP!" == "" (
    set DBNAME=
) else (
    call :LowTOUper !DBNAMETMP! DBNAME
)
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "MaxTime" MAXTIME
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "MinTime" MINTIME
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "MaxScn" MAXSCN
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "MinScn" MINSCN
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "dataPath" dataMount
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "logPath" logMount

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetRMANAuth
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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativeexpire.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativeexpire.bat %AGENT_ROOT% %PID%"
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

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;MAXTIME=!MAXTIME!;MINTIME=!MINTIME!;MAXSCN=!MAXSCN!;MINSCN=!MINSCN!"

if "!MINSCN!" == "" (
    echo DELETE FORCE NOPROMPT ARCHIVELOG UNTIL SCN !MAXSCN! LIKE 'EPOCH-SCN_%%\ARCH-S-%%'; > "%AGENT_TMP_PATH%\%EXECSQL%"
    echo DELETE FORCE NOPROMPT ARCHIVELOG UNTIL SCN !MAXSCN! LIKE 'EPOCH-SCN_%%/ARCH-S-%%'; >> "%AGENT_TMP_PATH%\%EXECSQL%"
) else  (
    echo DELETE FORCE NOPROMPT ARCHIVELOG FROM SCN !MINSCN! UNTIL SCN !MAXSCN! LIKE 'EPOCH-SCN_%%\ARCH-S-%%'; > "%AGENT_TMP_PATH%\%EXECSQL%"
    echo DELETE FORCE NOPROMPT ARCHIVELOG FROM SCN !MINSCN! UNTIL SCN !MAXSCN! LIKE 'EPOCH-SCN_%%/ARCH-S-%%'; >> "%AGENT_TMP_PATH%\%EXECSQL%"
)
echo exit >> "%AGENT_TMP_PATH%\%EXECSQL%"
call :Log "Exec rman to expire database log %DBINSTANCE%."
rem file can not support blank, when executing rman, when excuting, ship file name without directory path
call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!RMAN_LOGIN!" 3600 RetCode
if "!RetCode!" NEQ "0" (
    call :Log "Expire database log failed."
    set ERR_CODE=!RetCode!
    goto :error
)

call :Log "Expire database %DBINSTANCE% log successfully."
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

rem Convert str to upper
:LowTOUper
    set CONVERTSTR=%~1
    for %%a in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    set %~2=!CONVERTSTR!
goto :EOF

:SetRMANAuth
    if !AUTHMODE!==1 (
        set RMAN_LOGIN=/
    )
    if !AUTHMODE!==0 (
        set RMAN_LOGIN=%DBUSER%/"%DBUSERPWD%"
    )
goto :eof

:error
    call :DeleteFile "%AGENT_TMP_PATH%\%EXECSQL%"
    call :DeleteFile "%AGENT_TMP_PATH%\%EXECSQLRST%"
    exit /b %ERR_CODE%
endlocal
