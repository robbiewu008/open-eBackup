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
rem @dest:   application agent for oracle
rem @date:   2020-04-27
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

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclenativebackup.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryServiceRST%PID%.txt"
set ORACLEUSER=
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
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call :LowTOUper !DBNAME! DBNAME_UP
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
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "DataPath" DATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "LogPath" LOGPATH
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "MetaDataPath" METADATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Level" LEVEL
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Qos" QOS
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "dbType" DBTYPE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "storType" STORTYPE
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
    goto :end
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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativebackup.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativebackup.bat %AGENT_ROOT% %PID%"
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
set BACKUP_TEMP=!DATAPATH!\tmp
call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;ASMSIDNAME=!ASMSIDNAME!;ASMUSER=!ASMUSER!;CHANNELS=!CHANNELS!;LEVEL=!LEVEL!;QOS=!QOS!;BACKUP_TEMP=!BACKUP_TEMP!;DBTYPE=!DBTYPE!;STORTYPE=!STORTYPE!;DATAPATH=!DATAPATH!;LOGPATH=!LOGPATH!;METADATAPATH=!METADATAPATH!"
call :DeleteFile %RSTFILE%

where sqlplus
if !errorlevel! NEQ 0 (
    call :Log "can't find sqlplus"
    set ERR_CODE=1
    goto :end
)
where rman
if !errorlevel! NEQ 0 (
    call :Log "can't find rman"
    set ERR_CODE=1
    goto :end
)

rem set authrity about DATAPATH and LOGPATH
call :GetOracleUser

rem check parameter
if "!DATAPATH!" == "" (
    call :Log "DATAPATH is null"
    goto :error
)

if "!LOGPATH!" == "" (
    call :Log "LOGPATH is null"
    goto :error
)

if "!DBINSTANCE!" == "" (
    call :Log "DBINSTANCE is null"
    goto :error
)

if "!DATAPATH!" == "!LOGPATH!" (
    call :Log "DATAPATH is equal to LOGPATH"
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
if not "!LEVEL!" == "0" (
    if not "!LEVEL!" == "1" (
        not if "!LEVEL!" == "2" (
            set LEVEL=0
            call :Log "!LEVEL! is invalid, set level=0 full backup"
        )
    )
)

if not "%ASMSIDNAME%" == "" (
    rem try to get ASM instance name
    call sc query state= all | findstr /i "OracleASMService%ASMSIDNAME%" > %QUREYSERVICE%
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

set ADDITIONAL=
set "fstChar=!DATAPATH:~0,1!"
if "!fstChar!" == "+" (
    set ADDITIONAL=!METADATAPATH!\additional
    rem config authority about asm meta data path
    call icacls "%METADATAPATH%" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
    call :Log "Set %METADATAPATH% to !ORACLEUSER! full control."
    set SPLIT_CHAR=/
) else (
    set ADDITIONAL=!DATAPATH!\additional
    call :CreateDir "!ADDITIONAL!"
    
    rem config authority about data and log path
    call icacls "%DATAPATH%" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
    call icacls "%LOGPATH%" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
    call :Log "Set %DATAPATH% and %LOGPATH% to !ORACLEUSER! full control."
    set SPLIT_CHAR=\
)

rem set permission
set DataVol=
set LogVol=
call :GetVolByPath "%DATAPATH%" DataVol
call :GetVolByPath "%LOGPATH%" LogVol
call icacls !DataVol! /grant !ORACLEUSER!:F
call icacls !LogVol! /grant !ORACLEUSER!:F
call :Log "configure !DataVol! and !LogVol! to !ORACLEUSER! full control."

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
    if "!fstChar!" == "+" (
        set ERR_CODE=1
        goto :end
    )
)
call :Log "ORA_VERSION=!ORA_VERSION!;GRIDHOMEPATH=!GRIDHOMEPATH!;ORACLEBASEPATH=!ORACLEBASEPATH!;ORACLEHOMEPATH=!ORACLEHOMEPATH!"
rem create directory
call :CreateDir "!BACKUP_TEMP!"

call :DeleteFile "!ADDITIONAL!\ok"
call :Log "Begin to backup oracle"
rem test db instance
call :TestDBConnect
rem check archive mode
call :CheckArchiveMode
rem enable bct
call :EnableBCT

rem query dbid and db name
call :QueryDBID dbId dbUName
if "!dbId!" == "" (
    call :Log "can't get dbID"
    set ERR_CODE=1
    goto :end
)
if "!dbUName!" == "" (
    call :Log "can't get dbUName"
    set ERR_CODE=1
    goto :end
)

rem echo db information
rem query current incarnations
echo list incarnation; > %EXECSQL%
echo exit >> %EXECSQL%
call :Log "Begin to query database incarnations !DBINSTANCE!"
call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQLRMAN% %EXECSQLRMANRST% %DBINSTANCE% "!RMAN_LOGIN!" 30 Ret_Code
if "!Ret_Code!" EQU "0" (
    call :Log "Query database incarnations !DBINSTANCE! successfully."
) else (
    call :Log "Query database incarnations !DBINSTANCE! failed, ret !Ret_Code!."
    set ERR_CODE=!RetCode!
    goto :error
)
set INCAR_NUM=
for /f "tokens=2-5 delims= " %%a in ('type %EXECSQLRST%') do (
    if "%%d" == "CURRENT" (
        set INCAR_NUM=%%a
    )
)
echo #!dbId!;#!dbUName!;#!DBINSTANCE!;#!ASMUSER!;#!ASMSIDNAME!;#!INCAR_NUM! > "!ADDITIONAL!\dbinfo"
echo ORACLE_BASE=!ORACLEBASEPATH! > "!ADDITIONAL!\env_file"
echo ORACLE_HOME=!ORACLEHOMEPATH! >> "!ADDITIONAL!\env_file"
rem switch log
call :SwitchLog

rem get RESETLOGS_CHANGE#: NUMBER System change number (SCN) at open resetlogs
call :GetEpochSCN epochScn epochTimeStr epochTimeStamp
echo !epochScn! !epochTimeStr! !epochTimeStamp! > "!ADDITIONAL!\scn_epoch"
call :Log "scn_epoch:!epochScn! !epochTimeStr! !epochTimeStamp!"

rem get scn before backup
call :GetBeforeBackupSCN befScn befTimeStr befTimeStamp
echo !befScn! !befTimeStr! !befTimeStamp! > "!ADDITIONAL!\scn_before_backup"
call :Log "scn_before_backup:!befScn! !befTimeStr! !befTimeStamp!"

rem set specificSubdirs
call :SetSpecificSubdirs

rem get backup scn with huawei backup archive log
call :GetFromSCN from_scn
if "!from_scn!" == "" (
    set LEVEL=0
)

echo LEVEL=!LEVEL!
rem delete backup file when full backup
if "!LEVEL!" == "0" (
    call :DelFilesBeforeFullBackup
)

rem cross check dbf
call :CrossCheckDBF

rem backup database
call :BackupDatabase

rem get scn_dbf_max
call :GetDbfMaxSCN aftScn aftTimeStr aftTimeStamp
echo !aftScn! !aftTimeStr! !aftTimeStamp!> "!ADDITIONAL!\scn_dbf_max"
call :Log "scn_dbf_max:!aftScn! !aftTimeStr! !aftTimeStamp!"

rem get archive scn list
call :GetArchiveSCNList Sequence First_change First_time fisrtStamp Next_change Next_time nextStamp
echo !Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp! > "!ADDITIONAL!\archive_scn_list"
call :Log "archive_scn_list:!Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp!"

rem GetArchiveScn2DefuzzyList isn't used
rem GetArchiveScnAllList isn't used

rem update archive list
call :UpdateArchiveList

rem create json
call :CreateJson

rem get db control file list and spfile file
call :GetDBfiles 0
call :GetDBfiles 1

rem get filepath and tabelspace
call :GetFilePathAndTableSpace

rem get database log files and group
call :GetLogGroupIDAndFiles

rem copy some additional file
call :GenerateAdditionalInfo

rem write result file
echo databackuprst;!aftScn!;!aftTimeStamp!> !RSTFILE!
call :Log "databackuprst;!aftScn!;!aftTimeStamp!"
echo logbackuprst;!First_change!;!fisrtStamp!;!Next_change!;!nextStamp!>> !RSTFILE!
call :Log "logbackuprst;!First_change!;!fisrtStamp!;!Next_change!;!nextStamp!"
call :Log "do Backup %DBINSTANCE% successfully."
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
    set "CONVERTSTR=%~1"
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set "CONVERTSTR=%%CONVERTSTR:%%a=%%a%%"
    )
    set "%~2=!CONVERTSTR!"
goto :EOF

rem Convert str to upper
:LowTOUper
    set "CONVERTSTR=%~1"
    for %%a in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        call set "CONVERTSTR=%%CONVERTSTR:%%a=%%a%%"
    )
    set "%~2=!CONVERTSTR!"
goto :EOF

:GetOracleUser
    set ERR_CODE=1
    rem get oracle service owner
    set execUser=
    for /f "tokens=1,2 delims=:" %%a in ('sc qc "OracleService%DBNAME%"') do (
        echo %%a | findstr "SERVICE_START_NAME"
        if !errorlevel! equ 0 (
            set execUser=%%b
        )
    )

    if "!execUser!" == "" (
        call :Log "Get service OracleService%DBNAME% owner failed."
        goto :error
    )

    rem handle local user, local system and other, do nothing
    echo !execUser! | findstr ".\\" >nul
    if !errorlevel! NEQ 0 (
        call :Log "User !execUser!, don't config datapath and logpath."
        goto :eof
    )

    rem get owner sid and user caption
    set ownUserName=
    for /f "tokens=2 delims=\\" %%a in ("!execUser!") do (
        set ownUserName=%%a
    )
    if "!ownUserName!" == "" (
        call :Log "Get username from !execUser! failed."
        goto :error
    )

    for /f "tokens=1 delims= " %%a in ('wmic useraccount where name^=^"!ownUserName!^" get caption') do (
        echo %%a | findstr "!ownUserName!" >nul
        if !errorlevel! equ 0 (
            set ORACLEUSER=%%a
        )
    )
    if "!ORACLEUSER!" == "" (
        call :Log "Get username caption from !ownUserName! failed."
        goto :error
    )
goto :eof

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

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo set linesize 600 > %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select status from v$instance; >> %EXECSQL%
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get status of instance %DBINSTANCE%."
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
            call :CreateDir "!bctTag!"
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

:QueryDBID
    echo set linesize 100 > %EXECSQL%
    echo col db_unique_name for a30 >> %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select DBID, db_unique_name from v$database; >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get dbid and name of instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get instance dbid and name failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        for /f "tokens=1,2 delims= " %%a in ('type %EXECSQLRST%') do (
            set %~1=%%a
            set %~2=%%b
        )
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
    rem v$database:RESETLOGS_CHANGE#   NUMBER    System change number (SCN) at open resetlogs
    rem v$archived_log:RESETLOGS_CHANGE#	NUMBER	Resetlogs change number of the database when the log was written
    rem get the biggest scn by the lastest archive log, the log is from the reset log scn
    echo set linesize 100 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select bef_backup_change, to_char(scn_to_timestamp(bef_backup_change), %TIME_FORMAT%) from (select max(next_change#) as bef_backup_change from v$archived_log where (resetlogs_change# = (select resetlogs_change# from v$database)) and (deleted = 'NO')); >> %EXECSQL%
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

:DelFilesBeforeFullBackup
    call :Log "Begin to delete backup data before backup."
    if "!fstChar!" == "+" (
        set ORACLE_SID=!ASMSIDNAME!
        set ORACLE_HOME=!GRIDHOMEPATH!
        call asmcmd rm '!DATAPATH!\fno-*_ts-*.dbf'
    ) else (
        for /f %%a in ('dir /B "!DATAPATH!" ^| findstr "[FNO-].*[_TS-].*[.dbf]"') do (
            call :DeleteFile "!DATAPATH!\%%a"
        )
    )
goto :eof

:CrossCheckDBF
    call :Log "Begin to crocss check data file copy of database %DBNAME%."
    echo crosscheck datafilecopy tag 'EBACKUP-%DBNAME_UP%-DATA'; > %EXECSQL%
    echo exit >> %EXECSQL%
    call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQLRMAN% %EXECSQLRMANRST% %DBINSTANCE% "!RMAN_LOGIN!" 18000 Ret_Code
    if "!Ret_Code!" EQU "0" (
        call :Log "cross check data file copy successfully."
    ) else (
        call :Log "cross check data file copy failed, ret !Ret_Code!."
    )
goto :eof

:BackupDatabase
    echo configure backup optimization off; > %EXECSQL%
    echo configure controlfile autobackup off; >> %EXECSQL%
    echo set nocfau; >> %EXECSQL%
    echo configure maxsetsize to unlimited; >> %EXECSQL%
    echo configure encryption for database off; >> %EXECSQL%
    echo RUN >> %EXECSQL%
    echo { >> %EXECSQL%

    if "!LEVEL!" == "0" (
        set from_scn=!befScn!
        echo   change archivelog like '%%%SPLIT_CHAR%EPOCH-SCN_%%%SPLIT_CHAR%ARCH-S-%%' uncatalog; >> %EXECSQL%
    )

    rem set channel number
    for /l %%i in (1,1,!CHANNELS!) do (
        if !QOS! LEQ 0 (
            echo   allocate channel eBackup%%i type disk format '!DATAPATH!%SPLIT_CHAR%FNO-%%f_TS-%%N.dbf'; >> %EXECSQL%
        ) else (
            echo   allocate channel eBackup%%i type disk format '!DATAPATH!%SPLIT_CHAR%FNO-%%f_TS-%%N.dbf' rate !QOS!M; >> %EXECSQL%
        )
    )

    echo   configure device type disk parallelism !CHANNELS!; >> %EXECSQL%
    echo   backup spfile format '!DATAPATH!%SPLIT_CHAR%spfile.bs' tag 'EBACKUP-!DBNAME_UP!-SPFILE' reuse; >> %EXECSQL%
    echo   create pfile='!ADDITIONAL!%SPLIT_CHAR%ebackup-!DBNAME!-pfile.ora' from spfile; >> %EXECSQL%

    rem backup action
    if "!LEVEL!" == "0" (
        echo   backup as copy incremental level 0 tag 'EBACKUP-!DBNAME_UP!-DATA' database format '!DATAPATH!%SPLIT_CHAR%FNO-%%f_TS-%%N.dbf'; >> %EXECSQL%
    )
    if "!LEVEL!" == "1" (
        echo   backup incremental level 1 for recover of copy with tag 'EBACKUP-!DBNAME_UP!-DATA' database format '!BACKUP_TEMP!%SPLIT_CHAR%%%T_%%U'; >> %EXECSQL%
    )
    if "!LEVEL!" == "2" (
        echo   backup incremental level 1 cumulative for recover of copy with tag 'EBACKUP-!DBNAME_UP!-DATA' database format '!BACKUP_TEMP!%SPLIT_CHAR%%%T_%%U'; >> %EXECSQL%
    )

    echo   recover copy of database with tag 'EBACKUP-!DBNAME_UP!-DATA'; >> %EXECSQL%
    echo   delete noprompt backup tag 'EBACKUP-!DBNAME_UP!-DATA'; >> %EXECSQL%
    echo   backup as copy archivelog from scn !from_scn! format '!LOGPATH!%SPLIT_CHAR%EPOCH-SCN_!epochScn!%SPLIT_CHAR%ARCH-S-%%h-%%e-%%T-%%t.log' reuse; >> %EXECSQL%
    echo   backup as copy current controlfile format '!DATAPATH!%SPLIT_CHAR%CONTROLFILE.CTL' tag 'EBACKUP-!DBNAME_UP!-CTL' reuse; >> %EXECSQL%

    rem release channel
    for /l %%i in (1,1,!CHANNELS!) do (
        echo   release channel eBackup%%i; >> %EXECSQL%
    )
    echo } >> %EXECSQL%
    echo exit >> %EXECSQL%

    rem execute action
    call :Log "Begin to backup database !DBINSTANCE!"
    call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQLRMAN% %EXECSQLRMANRST% %DBINSTANCE% "!RMAN_LOGIN!" 18000 Ret_Code
    if "!Ret_Code!" EQU "0" (
        call :Log "Backup database !DBINSTANCE! successfully."
    ) else (
        call :Log "Backup database !DBINSTANCE! failed, ret !Ret_Code!."
        set ERR_CODE=!RetCode!
        goto :error
    )
goto :eof

:GetDbfMaxSCN
    rem get the biggest scn by the lastest backup archive log, the log is from the lastest huawei backup archive log
    echo set linesize 100 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select count(*) from v$datafile_copy where tag='EBACKUP-!DBNAME_UP!-DATA'; >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get the huawei backup number of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the huawei backup number failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        set /a bkNum=0
        for /f "tokens=* delims= " %%a in ('type %EXECSQLRST%') do (
            set /a bkNum=%%a
        )
        if "!bkNum!" EQU "0" (
            call :Log "no Huawei backup logging"
            set %~1=0
            set %~2=0
            set %~3=0
            goto :eof
        )
    )

    echo set linesize 300 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select scn, to_char(scn_to_timestamp(scn), %TIME_FORMAT%) from (select max(checkpoint_change#) as scn from v$datafile_copy where tag='EBACKUP-!DBNAME_UP!-DATA'); >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get the lastest huawei backup data file scn of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the lastest huawei backup data file scn failed."
        set ERR_CODE=!RetCode!
        goto :error
    ) else (
        set SCN=
        set timeStr=
        for /f "tokens=1,2 delims= " %%a in ('type %EXECSQLRST%') do (
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

:GetArchiveSCNList
    rem get the huawei backup archive log scn
    echo set linesize 300 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select p.* from (select sequence#, next_change#, to_char(next_time, %TIME_FORMAT%) from v$archived_log where !LOG_IS_BACKED_UP! and (next_change# ^> !from_scn!) order by sequence# desc)p where rownum=1; >> %EXECSQL%
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
    for /f "tokens=1-3 delims=	 " %%a in ('type %EXECSQLRST%') do (
        set Sequence=%%a
        set Next_change=%%b
        set Next_time=%%c
    )

    echo set linesize 300 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select p.* from (select first_change#, to_char(first_time, %TIME_FORMAT%) from v$archived_log where !LOG_IS_BACKED_UP! and (next_change# ^> !from_scn!) order by sequence# asc)p where rownum=1; >> %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to get the 1st huawei backup archive log file scn of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the 1st huawei backup archive log file scn failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    for /f "tokens=1-2 delims=	 " %%a in ('type %EXECSQLRST%') do (
        set First_change=%%a
        set First_time=%%b
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

:UpdateArchiveList
    rem get epoch scn
    call :Log "update archive log list"
    set epochScn=
    for /f "tokens=1 delims= " %%a in ('type "!ADDITIONAL!\scn_epoch"') do (
        set epochScn=%%a
    )

    if "!fstChar!" == "+" (
        set ORACLE_SID=!ASMSIDNAME!
        set ORACLE_HOME=!GRIDHOMEPATH!
        call asmcmd ls "!LOGPATH!/EPOCH-SCN_!epochScn!/*" > %EXECSQLRST%
        for /f %%a in ('type %EXECSQLRST%') do (
            rem update file content
            echo %%a !Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp! > "!ADDITIONAL!\archive_scn_list"
            call :Log "%%a !Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp!"
        )
    ) else (
        for /f %%a in ('dir /B "!LOGPATH!\EPOCH-SCN_!epochScn!\*" ^| findstr "[arch-S-]!Sequence![-].*"') do (
            rem update file content
            echo %%a !Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp! > "!ADDITIONAL!\archive_scn_list"
            call :Log "%%a !Sequence! !First_change! !First_time! !fisrtStamp! !Next_change! !Next_time! !nextStamp!"
        )
    )
goto :eof

:CreateJson
    call :Log "CreateJson"
goto :eof

:GetDBfiles
    set queryType=%~1
    rem get the huawei backup archive log scn
    echo set linesize 600 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%

    if "!queryType!" == "0" (
        call :Log "Begin to query control file list."
        echo col name for a513 >> %EXECSQL%
        echo select name from v$controlfile; >> %EXECSQL%
    )

    if "!queryType!" == "1" (
        call :Log "Begin to query spfile file."
        echo col VALUE for a513 >> %EXECSQL%
        echo select VALUE from v$parameter where name='spfile'; >> %EXECSQL%
    )

    echo exit >> %EXECSQL%
    call :Log "Exec sql to get the file list of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the file list failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    if "!queryType!" == "0" (
        call copy /y %EXECSQLRST% "!ADDITIONAL!\ctrlfiles"
    )

    if "!queryType!" == "1" (
        call copy /y %EXECSQLRST% "!ADDITIONAL!\spfile"
    )
goto :eof

:GetFilePathAndTableSpace
    set MAIN_VERSION=%ORA_VERSION:~0,2%

    rem get the huawei backup archive log scn
    echo set linesize 999 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%

    if "!MAIN_VERSION!" GEQ "12" (
        echo col tsName for a30 >> %EXECSQL%
        echo col tsFile for a520 >> %EXECSQL%
        echo SELECT t.CON_ID CON_ID, '*', t.Name tsName, '*', f.File# fNo, '*', f.Name tsFile FROM V$TABLESPACE t, V$DATAFILE f WHERE t.TS# = f.TS# and t.CON_ID=f.CON_ID ORDER BY t.CON_ID; >> %EXECSQL%
    ) else (
        echo col TABLESPACE_NAME for a30 >> %EXECSQL%
        echo col FILE_NAME for a520 >> %EXECSQL%
        echo select 0, '*', TABLESPACE_NAME, '*', FILE_ID, '*', FILE_NAME from dba_data_files; >> %EXECSQL%
    )
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get the tablespace and file name of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the tablespace and file name failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    call :DeleteFile "!ADDITIONAL!\dbfiles"
    for /f "tokens=1-5 delims=*	" %%a in ('type %EXECSQLRST%') do (
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
            set line=!line!!str!;
        )
        call :TrimString "%%e" str
        if not "!str!" == "" (
            set line=!line!!str!;
        )
        
        echo !line! >> "!ADDITIONAL!\dbfiles"
    )

    call :Log "Get the tablespace and file name successfully."
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
goto :eof

:RightTrim
if "%trimString:~-1%"==" " (
    set "trimString=%trimString:~0,-1%"
    goto RightTrim
)
goto :eof

:GetLogGroupIDAndFiles
    rem get the huawei backup archive log scn
    echo set linesize 300 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo col MEMBER for a255 >> %EXECSQL%
    echo select GROUP#, MEMBER from v$logfile order by GROUP#; >> %EXECSQL%
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get the log file list of instance !DBINSTANCE!."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get the log file list failed."
        set ERR_CODE=!RetCode!
        goto :error
    )

    call :DeleteFile "!ADDITIONAL!\logfiles"
    for /f "tokens=1,2 delims=	 " %%a in ('type %EXECSQLRST%') do (
        echo %%a;%%b >> "!ADDITIONAL!\logfiles"
    )
    call :Log "Get the log file list successfully."
goto :eof

:GenerateAdditionalInfo
    if not exist "!ADDITIONAL!\dbs" (
        md "!ADDITIONAL!\dbs"
        call icacls "!ADDITIONAL!\dbs" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
    )
    
    if not exist "!ADDITIONAL!\netadmin" (
        md "!ADDITIONAL!\netadmin"
        call icacls "!ADDITIONAL!\netadmin" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
    )

    call xcopy /y/s/e "!ORACLEHOMEPATH!\dbs\init*.ora" "!ADDITIONAL!\dbs"

    set SpfileName=
    for /f "tokens=2 delims=:" %%a in ('call "!ORACLEHOMEPATH!\bin\srvctl" config database -d !DBNAME! ^| findstr Spfile') do (
        set SpfileName=%%a
    )

    set pwfileName=
    for /f "tokens=2 delims=:" %%a in ('call "!ORACLEHOMEPATH!\bin\srvctl" config database -d !DBNAME! ^| findstr "Password^ file"') do (
        set pwfileName=%%a
    )

    if "!pwfileName!" == "" (
        set pwfileName="!ORACLEHOMEPATH!\dbs\orapw!DBINSTANCE!"
    )

    if not "!SpfileName!" == "" (
        call :CopyFile2Backup "!SpfileName!" "!ADDITIONAL!\dbs"
    )

    if not "!pwfileName!" == "" (
        call :CopyFile2Backup "!pwfileName!" "!ADDITIONAL!\dbs"
    )

    call xcopy /y/s/e !ORACLEHOMEPATH!\network\admin\* "!ADDITIONAL!\netadmin"
    rem backup successfully
    echo. > "!ADDITIONAL!\ok"
goto :eof

:CopyFile2Backup
    set sourceFile=%~1
    set tgtDir=%~2

    set fileType=!sourceFile:~0,1!
    if "!fileType!" == "+" (
        set ORACLE_SID=!ASMSIDNAME!
        set ORACLE_HOME=!GRIDHOMEPATH!
        call asmcmd cp "!sourceFile!" "!tgtDir!"
    ) else (
        if not exist "!sourceFile!" (
            call :Log "!sourceFile! is not exists."
            goto :eof
        )
        call copy /y "!sourceFile!" "!tgtDir!"
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
            if !errorlevel! NEQ 0 (
                call :Log "create fs folder !DirName! failed."
                set ERR_CODE=1
                goto :error
            )

            call icacls "!DirName!" /grant "!ORACLEUSER!":^(OI^)^(CI^)F
        )
    )
goto :eof

:getSubDir
    set dirName=%2
    for /f "tokens=%1 delims=/" %%x in ("%dirName%") do (
        set %2=%%x
    )
goto :eof

:GetVolByPath
    set GetVolFile=%AGENT_TMP_PATH%GetVolbyPath%PID%
    call :DeleteFile "!GetVolFile!"
    call "%POWERSHELL_PATH%\powershell.exe" -command ".\agentcom.ps1 'GetVolByPath' '%~1' '%GetVolFile%'"

    set VolPath=
    for /f %%a in ('type "!GetVolFile!"') do (
        if not "%%a" == "" (
            set VolPath=%%a
        )
    )
    call :DeleteFile "!GetVolFile!"
    if "!VolPath!" == "" (
            call :Log "Can't find vol by path %~1."
            set ERR_CODE=1
            goto :error
    )
    set %~2=!VolPath!
goto :eof

:error
    call :DeleteFile %EXECSQL%
    call :DeleteFile %EXECSQLRST%
    exit %ERR_CODE%
endlocal
