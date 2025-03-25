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
rem @date:   2020-04-29
rem @authr:  
rem @modify:

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERROR_RECOVER_INSTANCE_NOSTART=11
set /a ERR_FILE_IS_EMPTY=8
set /a ERROR_ORACLE_NOARCHIVE_MODE=24

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"
set CMD_GETVALUE=getvalue
set CMD_EXECSQL_SILE=execsqls
set CMD_EXECASMSQL_SIL=execasmsqls
set TIME_FORMAT='YYYY-MM-DD_HH24:MI:SS'
set SEPARATOR=      -       

rem check instance status
set EXECSQL="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set EXECSQLRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"
set EXECSQLRMAN="CheckInstanceStatus%PID%.sql"
set EXECSQLRMANRST="CheckInstanceStatusRST%PID%.txt"
set ARCHIVE_DEST_LIST="%AGENT_TMP_PATH%ArchiveDestList%PID%.txt"
set ARCHIVE_DIR="%AGENT_TMP_PATH%ArchiveDir%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclenativearchiveback.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryServiceRST%PID%.txt"
set SPLIT_CHAR=\
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
call :LowTOUper !DBNAME! DBNAME_UP
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMSIDNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Channel" CHANNELS
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "LogPath" LOGPATH
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "truncateLog" TRUNCATELOG
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Qos" QOS
set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetDBAuth
call :SetASMAuth
call :SetRMANAuth

call :log "Begin to get powershell path."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1\PowerShellEngine" /v "ApplicationBase"') do set "POWERSHELL_PATH=%%k" 
if "!POWERSHELL_PATH!"=="" (
    call :log "Get powershell path faild."
    set ERR_CODE=%ERR_FILE_IS_EMPTY%
    goto :error
)
call :log "Get powershell path succ, powershell path %POWERSHELL_PATH%."

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativearchiveback.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativearchiveback.bat %AGENT_ROOT% %PID%"
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
set ADDITIONAL=
set fstChar=!DATAPATH:~0,1!
if "!fstChar!" == "+" (
    set ADDITIONAL=!METADATAPATH!\additional
    set SPLIT_CHAR=/
) else (
    set ADDITIONAL=!DATAPATH!\additional
)
set BACKUP_TEMP=!LOGPATH!\tmp
call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;ASMSIDNAME=!ASMSIDNAME!;ASMUSER=!ASMUSER!;CHANNELS=!CHANNELS!;LOGPATH=!LOGPATH!;TRUNCATELOG=!TRUNCATELOG!;QOS=!QOS!;BACKUP_TEMP=!BACKUP_TEMP!"
call :DeleteFile %RSTFILE%

rem check parameter
if "!LOGPATH!" == "" (
    call :Log "LOGPATH is null"
    goto :error
)
if "!DBINSTANCE!" == "" (
    call :Log "DBINSTANCE is null"
    goto :error
)

rem set default params if not exist
if "!CHANNELS!" == "" (
    call :Log "Setting channels number to $CHANNELS by default"
    set /a CHANNELS=4
) else (
    if "!CHANNELS!" == "0" (
        call :Log "Setting channels number to $CHANNELS by default"
        set /a CHANNELS=4
    )
)

if "!fstChar!" == "+" (
    rem try to get ASM instance name
    sc query state= all | findstr /i "OracleASMService%ASMSIDNAME%" > %QUREYSERVICE%
    set ASMSIDNAME=+ASM
    for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
        if "SERVICE_NAME"=="%%a" (
            for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
                set ASMSIDNAME=+%%j
            )
        )
    )
    call :DeleteFile %QUREYSERVICE%
    call :Log "check ASM Instance=!ASMSIDNAME!"
)

set nls_lang=AMERICAN_AMERICA.ZHS16GBK
rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION
rem ************************get oracle path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETORAPATH% %PID% %LOGFILE% ORACLEBASEPATH ORACLEHOMEPATH
if "!ORACLEBASEPATH!" == "" (
    call :Log "Get Oracle base path failed." 
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :eof
)
if "!ORACLEHOMEPATH!" == "" (
    call :Log "Get Oracle home path failed." 
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :eof
)
rem ************************get grid path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETGRIDPATH% %PID% %LOGFILE% !ORA_VERSION! GRIDHOMEPATH
if "!GRIDHOMEPATH!" == "" (
    call :Log "Get grid home path failed." 
    set ERR_CODE=1
    goto :error
)
call :Log "ORA_VERSION=!ORA_VERSION!;GRIDHOMEPATH=!GRIDHOMEPATH!;ORACLEBASEPATH=!ORACLEBASEPATH!;ORACLEHOMEPATH=!ORACLEHOMEPATH!"

rem create directory
call :CreateDir "!BACKUP_TEMP!"

where sqlplus
if !errorlevel! NEQ 0 (
    call :Log "can't find sqlplus"
    set ERR_CODE=1
    goto :error
)
where rman
if !errorlevel! NEQ 0 (
    call :Log "can't find rman"
    set ERR_CODE=1
    goto :error
)

call :Log "Begin to backup oracle"
rem test db instance
call :TestDBConnect
rem check archive mode
call :CheckArchiveMode
rem enable bct
call :EnableBCT

rem switch log
call :SwitchLog

rem get RESETLOGS_CHANGE#: NUMBER System change number (SCN) at open resetlogs
call :GetEpochSCN epochScn epochTimeStr epochTimeStamp
call :Log "scn_epoch:!epochScn! !epochTimeStr! !epochTimeStamp!"

rem set specificSubdirs
call :SetSpecificSubdirs

rem get scn before backup
if "!TRUNCATELOG!" == "1" (
    call :GetBeforeBackupSCN befScn befTimeStr befTimeStamp
    call :Log "scn_before_backup:!befScn! !befTimeStr! !befTimeStamp!"
)

rem get backup scn with huawei backup archive log
call :GetFromSCN from_scn

rem backup archive log database
if "!from_scn!" == "" (
    call :Log "There are no archive log backup by huawei, don't need to backup archive log."
    set ERR_CODE=1
    goto :error
)
call :BackupLog

if "!TRUNCATELOG!" == "1" (
   rem call :DoTrunCateLog !befScn!
)

rem get archive scn list
call :GetArchiveSCNList Sequence First_change First_time fisrtStamp Next_change Next_time nextStamp
call :Log "archive_scn_list:!Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp!"

rem write result file
echo databackuprst;;> !RSTFILE!
echo logbackuprst;!First_change!;!fisrtStamp!;!Next_change!;!nextStamp!>> !RSTFILE!
call :Log "logbackuprst;!First_change!;!fisrtStamp!;!Next_change!;!nextStamp!"
call :Log "do Backup %DBINSTANCE% log file successfully."
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
    set "CONVERTSTR=%~1"
    for %%a in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        call set "CONVERTSTR=%%CONVERTSTR:%%a=%%a%%"
    )
    set "%~2=!CONVERTSTR!"
goto :EOF

:TestDBConnect
    rem check asm intance status
    call :Log "Start to check oracle instance status %DBINSTANCE%."
    call :GetInstanceStatus INSTStatus RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get instance status failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    rem STARTED - After STARTUP NOMOUNT
    rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    rem OPEN - After STARTUP or ALTER DATABASE OPEN
    rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if not "!INSTStatus:~0,4!" == "OPEN" (
        call :Log "Instance status !INSTStatus! no open"
        set ERR_CODE=%ERROR_RECOVER_INSTANCE_NOSTART%
        goto :error
    )
    call :Log "Test %DBNAME% successful."
    goto :eof

:CheckArchiveMode
    echo set linesize 600 > %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select LOG_MODE from v$database; >> %EXECSQL%
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get log mode of instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get logmode of instance failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    set DBLogMode=
    for /f %%a in ('type %EXECSQLRST%') do (
        set DBLogMode=%%a
    )

    if "!DBLogMode!" == "NOARCHIVELOG" (
        call :Log "database !DBINSTANCE! isn't log mode"
        set ERR_CODE=ERROR_ORACLE_NOARCHIVE_MODE
        goto :error
    )
    call :Log "Get database log mode successfully"
goto :eof

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo set linesize 600 > %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select status from v$instance; >> %EXECSQL%
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get status of instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        set /a %~2 = !RetCode!
    ) else (
        rem STARTED - After STARTUP NOMOUNT
        rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        rem OPEN - After STARTUP or ALTER DATABASE OPEN
        rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        for /f "delims=" %%a in ('type %EXECSQLRST%') do (
            if not "%%a" == "" (
                set %~1=%%a
                set /a %~2 = 0
            )
        )
    )
goto :eof

:EnableBCT
    rem get BCT status
    set bctStatus=
    echo set heading off > %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select status from v$block_change_tracking; >> %EXECSQL%
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get bct status of instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get instance bct status failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        for /f %%a in ('type %EXECSQLRST%') do (
            set bctStatus=%%a
        )
    )

    if not "!bctStatus!" == "ENABLED" (
        rem get bct tag value
        set bctTag=
        echo set heading off > %EXECSQL%
        echo set pagesize 0 >> %EXECSQL%
        echo set feedback off >> %EXECSQL%
        echo select value from v$parameter where name='db_create_file_dest'; >> %EXECSQL%
        echo exit >> %EXECSQL%
        call :Log "Exec sql to get bct tag of instance."
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
        if "!RetCode!" NEQ "0" (
            call :Log "Get instance bct tag failed."
            set ERR_CODE=!RetCode!
            goto :error
        ) else (
            for /f %%a in ('type %EXECSQLRST%') do (
                set bctTag=%%a
            )
        )

        rem set bct value
        if "!bctTag!" == "" (
            set bctTag=!ORACLEBASEPATH!\%DBINSTANCE%
            if not exist "!bctTag!" (
                md "!bctTag!"
                call icacls "!bctTag!" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
            )
            call :Log "bctTag=!bctTag!, grant !ORACLEUSER! authority."
        )

        echo alter database enable block change tracking using file '!bctTag!\rman_change_track.f' reuse; > %EXECSQL%
        echo exit >> %EXECSQL%
        call :Log "Exec sql to set bct tag of instance !bctTag!"
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
        if "!RetCode!" NEQ "0" (
            call :Log "Set instance bct tag failed."
            set ERR_CODE=!RetCode!
            goto :error
        )
        call :Log "set %DBINSTANCE% BCT tag !bctTag!"
    ) else (
        call :Log "%DBINSTANCE% BCT is already enabled"
    )
goto :eof

:SwitchLog
    echo alter system archive log current; > %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to switch online log."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Switch online log failed."
        set ERR_CODE=!RetCode!
        goto :error
    )
    call :Log "switch online log successfully."
goto :eof

:GetEpochSCN
    echo set linesize 100 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select resetlogs_change#, to_char(resetlogs_time, %TIME_FORMAT%) from v$database; >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get resetlogs change scn of instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get resetlogs change scn failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        set SCN=
        set timeStr=
        for /f "tokens=2-3 delims= " %%a in ('type %EXECSQLRST%') do (
            set SCN=%%a
            set timeStr=%%b
        )
        call "%POWERSHELL_PATH%\powershell.exe" -command "(([datetime]::parseexact('!timeStr!', 'yyyy-MM-dd_HH:mm:ss', $null).ToUniversalTime().Ticks - 621355968000000000)/10000000).tostring().Substring(0,10)" > %EXECSQLRST%
        set timeStamp=
        for /f %%a in ('type %EXECSQLRST%') do (
            set timeStamp=%%a
        )
        set %~1=!SCN!
        set %~2=!timeStr!
        set %~3=!timeStamp!
    )
goto :eof

:GetBeforeBackupSCN
    call :LowTOUper "!LOGPATH!" LOGPATH_UP
    rem v$database:RESETLOGS_CHANGE#   NUMBER    System change number (SCN) at open resetlogs
    rem v$archived_log:RESETLOGS_CHANGE#	NUMBER	Resetlogs change number of the database when the log was written
    rem get the biggest scn by the lastest archive log, the log is from the reset log scn
    echo set linesize 100 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select bef_backup_change, to_char(scn_to_timestamp(bef_backup_change), %TIME_FORMAT%) from (select max(next_change#) as bef_backup_change from v$archived_log where !LOG_IS_BACKED_UP!); >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get current scn and timestamp of instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get current scn and timestamp failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        set SCN=
        set timeStr=
        for /f "tokens=2,3 delims= " %%a in ('type %EXECSQLRST%') do (
            set SCN=%%a
            set timeStr=%%b
        )
        call "%POWERSHELL_PATH%\powershell.exe" -command "(([datetime]::parseexact('!timeStr!', 'yyyy-MM-dd_HH:mm:ss', $null).ToUniversalTime().Ticks - 621355968000000000)/10000000).tostring().Substring(0,10)" > %EXECSQLRST%
        set timeStamp=
        for /f %%a in ('type %EXECSQLRST%') do (
            set timeStamp=%%a
        )
        set %~1=!SCN!
        set %~2=!timeStr!
        set %~3=!timeStamp!
    )
goto :eof

:SetSpecificSubdirs
    call :CreateDir "%LOGPATH%"
    call :CreateDir "%LOGPATH%!SPLIT_CHAR!EPOCH-SCN_!epochScn!"
    call :LowTOUper "!LOGPATH!" LOGPATH_UP
    set "LOG_IS_BACKED_UP=(resetlogs_change# = (select resetlogs_change# from v$database)) and (deleted = 'NO') and name like '%LOGPATH_UP%!SPLIT_CHAR!EPOCH-SCN_%%!SPLIT_CHAR!ARCH-S-%%'"
goto :eof

:GetFromSCN
    rem get the biggest scn by the lastest backup archive log, the log is from the lastest huawei backup archive log
    echo set linesize 100 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select max(next_change#) from v$archived_log where !LOG_IS_BACKED_UP!; >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get the lastest huawei backup scn and timestamp of instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the lastest huawei backup scn and timestamp failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        set %~1=
        for /f "tokens=2 delims= " %%a in ('type %EXECSQLRST%') do (
            set %~1=%%a
        )
    )
goto :eof

:DoTrunCateLog
    rem do truncate log
    set befScn=%~1
    echo crosscheck archivelog all; > %EXECSQL%
    for /f %%i in ('type %ARCHIVE_DIR%') do (
        echo delete force noprompt archivelog until scn !befScn! like '%%i!SPLIT_CHAR!%%'; >> %EXECSQL%
    )
    echo exit >> %EXECSQL%
    call :Log "Exec sql to truncate log of instance %DBINSTANCE%, before !befScn!."
    call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!RMAN_LOGIN!" 36000 Ret_Code
    if "!RetCode!" NEQ "0" (
        call :Log "truncate log failed."
        set ERR_CODE=!RetCode!
        goto :error
    )
    call :Log "truncate %DBINSTANCE% log successfully."
goto :eof

:GetArchiveDestLst
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo set linesize 300; >> %EXECSQL%
    echo col DESTINATION for a255; >> %EXECSQL%
    echo ALTER SESSION SET NLS_LANGUAGE='AMERICAN'; >> %EXECSQL%
    echo select DESTINATION from v$archive_dest where STATUS='VALID'; >> %EXECSQL%
    echo exit >> %EXECSQL%
    
    call :Log "Execute sql to get destination of database."
    call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
    call :DeleteFile %EXECSQL%
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get archive dest list failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    REN %EXECSQLRST% !ARCHIVE_DEST_LIST!
    call :DeleteFile %ARCHIVE_DIR%
    for /f %%i in ('type %ARCHIVE_DEST_LIST%') do (
        set STRARCHIVEDEST=%%i
        call :Log "Dest=!STRARCHIVEDEST!"
        if "!STRARCHIVEDEST!" == "USE_DB_RECOVERY_FILE_DEST" (
            echo set pagesize 0 > %EXECSQL%
            echo set feedback off >> %EXECSQL%
            echo set linesize 300; >> %EXECSQL%
            echo col NAME for a255; >> %EXECSQL%
            echo select NAME from V$RECOVERY_FILE_DEST; >> %EXECSQL%
            echo exit >> %EXECSQL%
            
            call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
            if !RetCode! NEQ 0 (
                call :Log "Excute sql script for get archive dest failed."
                set ERR_CODE=!RetCode!
                goto :error
            )
            for /f %%k in ('type %EXECSQLRST%') do (
                echo %%k >> %ARCHIVE_DIR%
            )
        ) else (
            echo %%i >> %ARCHIVE_DIR%
        )
    )
    call :DeleteFile %ARCHIVE_DEST_LIST%
goto :eof

:BackupLog
    echo configure backup optimization off; > %EXECSQL%
    echo configure controlfile autobackup off; >> %EXECSQL%
    echo set nocfau; >> %EXECSQL%
    echo configure maxsetsize to unlimited; >> %EXECSQL%
    echo configure encryption for database off; >> %EXECSQL%
    echo RUN >> %EXECSQL%
    echo { >> %EXECSQL%

    rem set channel number
    for /l %%i in (1,1,!CHANNELS!) do (
        if "!QOS!" == "" (
            echo   allocate channel eBackup%%i type disk format '!LOGPATH!%SPLIT_CHAR%EPOCH-SCN_!epochScn!%SPLIT_CHAR%ARCH-S-%%e-%%T-%%t.log'; >> %EXECSQL%
        ) else (
            echo   allocate channel eBackup%%i type disk format '!LOGPATH!%SPLIT_CHAR%EPOCH-SCN_!epochScn!%SPLIT_CHAR%ARCH-S-%%e-%%T-%%t.log' rate %QOS%; >> %EXECSQL%
        )
    )

    echo   configure device type disk parallelism !CHANNELS!; >> %EXECSQL%
    echo   backup as copy archivelog from scn !from_scn! format '!LOGPATH!%SPLIT_CHAR%EPOCH-SCN_!epochScn!%SPLIT_CHAR%ARCH-S-%%h-%%e-%%T-%%t.log' reuse; >> %EXECSQL%

    rem release channel
    for /l %%i in (1,1,!CHANNELS!) do (
        echo   release channel eBackup%%i; >> %EXECSQL%
    )
    echo } >> %EXECSQL%
    echo exit >> %EXECSQL%

    rem execute action
    call :Log "Begin to backup database !DBINSTANCE! log file"
    call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQLRMAN% %EXECSQLRMANRST% %DBINSTANCE% "!RMAN_LOGIN!" 36000 Ret_Code
    if "!Ret_Code!" EQU "0" (
        call :Log "Backup database !DBINSTANCE! log file successfully."
    ) else (
        call :Log "Backup database !DBINSTANCE! log file failed, ret !Ret_Code!."
        set ERR_CODE=!RetCode!
        goto :error
    )
goto :eof

:GetArchiveSCNList
    rem get the huawei backup archive log scn
    echo set linesize 300 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select sequence#, first_change#, to_char(first_time, %TIME_FORMAT%), next_change#, to_char(next_time, %TIME_FORMAT%) from v$archived_log where !LOG_IS_BACKED_UP! and (next_change# ^> !from_scn!) order by sequence# desc; >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get the lastest huawei backup archive log file scn of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the lastest huawei backup archive log file scn failed."
        set ERR_CODE=!RetCode!
        goto :error
    )
    set Sequence=
    set First_change=
    set First_time=
    set Next_change=
    set Next_time=
    set /a line=1
    for /f "tokens=1-5 delims=	 " %%a in ('type %EXECSQLRST%') do (
        if !line! EQU 1 (
            set Sequence=%%a
            set Next_change=%%d
            set Next_time=%%e
        ) else (
            set First_change=%%b
            set First_time=%%c
        )
        set /a line=!line! + 1
    )

    set fisrtStamp=
    call "%POWERSHELL_PATH%\powershell.exe" -command "(([datetime]::parseexact('!First_time!', 'yyyy-MM-dd_HH:mm:ss', $null).ToUniversalTime().Ticks - 621355968000000000)/10000000).tostring().Substring(0,10)" > %EXECSQLRST%
    for /f %%a in ('type %EXECSQLRST%') do (
        set fisrtStamp=%%a
    )

    set nextStamp=
    call "%POWERSHELL_PATH%\powershell.exe" -command "(([datetime]::parseexact('!Next_time!', 'yyyy-MM-dd_HH:mm:ss', $null).ToUniversalTime().Ticks - 621355968000000000)/10000000).tostring().Substring(0,10)" > %EXECSQLRST%
    for /f %%a in ('type %EXECSQLRST%') do (
        set nextStamp=%%a
    )

    set %~1=!Sequence!
    set %~2=!First_change!
    set %~3=!First_time!
    set %~4=!fisrtStamp!
    set %~5=!Next_change!
    set %~6=!Next_time!
    set %~7=!nextStamp!
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

:SetASMAuth
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=%DBUSER%/"%DBUSERPWD%" as sysasm
    )
goto :eof

:SetRMANAuth
    if !AUTHMODE!==1 (
        set RMAN_LOGIN=/
    )
    if !AUTHMODE!==0 (
        set RMAN_LOGIN=%DBUSER%/"%DBUSERPWD%"
    )
goto :eof

:CreateDir
    rem only support one level directory
    set "DirName=%~1"
    set "dirFst=!DirName:~0,1!"
    if "!dirFst!" == "+" (
        set ORACLE_SID=!ASMSIDNAME!
        set ORACLE_HOME=!GRIDHOMEPATH!

        rem create diretory one by one
        set fullDir=
        set "tmpDir=!DirName!"
        rem support 10 level directory
        for /l %%a in (1 1 10) do (
            call :getSubDir %%a tmpDir
            if "!tmpDir!" == "" (
                goto :eof
            )
            
            if "!fullDir!" == "" (
                set fullDir=!tmpDir!
            ) else (
                set fullDir=!fullDir!/!tmpDir!
            )
            call asmcmd ls "!fullDir!" >> !LOGFILEPATH!
            if !errorlevel! NEQ 0 (
                call asmcmd mkdir "!fullDir!" >> !LOGFILEPATH!
                if !errorlevel! NEQ 0 (
                    call :Log "create asm folder !DirName! failed."
                    set ERR_CODE=1
                    goto :error
                )
            )
        )
    ) else (
        if not exist "!DirName!" (
            md "!DirName!"
        )
        
        if !errorlevel! NEQ 0 (
            call :Log "create fs folder !DirName! failed."
            set ERR_CODE=1
            goto :error
        )
    )
goto :eof

:error
    call :DeleteFile %EXECSQL%
    call :DeleteFile %EXECSQLRST%
    call :DeleteFile %ARCHIVE_DIR%
    call :DeleteFile %ARCHIVE_DEST_LIST%
    exit %ERR_CODE%

endlocal
