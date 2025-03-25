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

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

set GETDBINFO="%AGENT_TMP_PATH%GETDBINFO%PID%.sql"
set GETDBINFORST="%AGENT_TMP_PATH%GETDBINFORST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclestoredetail.log
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

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclestoredetail.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclestoredetail.bat %AGENT_ROOT% %PID%"
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

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!"
call :DeleteFile %RSTFILE%

call :Log "Begin to get oracle db detail."

rem check db status
call :TestDBConnect

rem get detail
call :GetDBFSType

call :DeleteFile %GETDBINFO%
call :DeleteFile %GETDBINFORST%
call :Log "Finish get oracle db detail."
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

rem get db storage type
:CreateGetStorageList
    echo set linesize 600 > "%~1"
    echo set pagesize 0 >> "%~1"
    echo set feedback off >> "%~1"
    echo select name from v$datafile; >> "%~1"
    echo select MEMBER from v$logfile; >> "%~1"
    echo select name from v$controlfile; >> "%~1"
    echo select file_name from dba_temp_files; >> "%~1"
    echo select VALUE from v$parameter where name='spfile'; >> "%~1"
    echo exit >> "%~1"
    goto :EOF

:GetDBFSType
    call :CreateGetStorageList %GETDBINFO% "%~1"
    call :Log "Exec sql to get storage file list of database."
    set DBTYPE=

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %GETDBINFO% %GETDBINFORST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Execute Script to query DB %DBNAME% file list failed."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :error
    ) else (
        for /f %%i in ('type %GETDBINFORST%') do (
            set LINE=%%i
            if not !LINE! == "" (
                set FSTCHAR=!LINE:~0,1!
                if "!FSTCHAR!" == "+" (
                    set DBTYPE=ASM
                ) else (
                    set DBTYPE=NTFS
                )

                call :Log "DBTYPE=!DBTYPE!"
                echo !DBTYPE! >> %RSTFILE%
                goto :eof
            )
        )
    )
    goto :eof

:TestDBConnect
    rem check db exists
    sc query %ORACLEDBSERVICE%
    if !errorlevel! NEQ 0 (
        call :Log "The %ORACLEDBSERVICE% Service isn't exists."
        exit 1
    )
	
    rem check asm intance status
    call :Log "Start to check oracle instance status."
    call :GetInstanceStatus INSTStatus RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get instance status failed."
        exit !RetCode!
    )

    rem STARTED - After STARTUP NOMOUNT
    rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    rem OPEN - After STARTUP or ALTER DATABASE OPEN
    rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if not "!INSTStatus:~0,4!" == "OPEN" (
        call :Log "Instance status !INSTStatus! no open"
        exit %ERROR_RECOVER_INSTANCE_NOSTART%
    )
    call :Log "Test %DBNAME% successful."
    goto :eof

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo set linesize 600 > %CHECKINSTANCESTATUS%
    echo set pagesize 0 >> %CHECKINSTANCESTATUS%
    echo set feedback off >> %CHECKINSTANCESTATUS%
    echo select status from v$instance; >> %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

    call :Log "Exec sql to get status of instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        set /a %~2 = !RetCode!
    ) else (
        rem STARTED - After STARTUP NOMOUNT
        rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        rem OPEN - After STARTUP or ALTER DATABASE OPEN
        rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        for /f "delims=" %%a in ('type %CHECKINSTANCESTATUSRST%') do (
            if not "%%a" == "" (
                set %~1=%%a
                set /a %~2 = 0
            )
        )
    )
    call :DeleteFile %CHECKINSTANCESTATUS%
    call :DeleteFile %CHECKINSTANCESTATUSRST%

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

:error
    call :DeleteFile %GETDBINFO%
    call :DeleteFile %GETDBINFORST%
    exit /b %ERR_CODE%
goto :eof

endlocal
