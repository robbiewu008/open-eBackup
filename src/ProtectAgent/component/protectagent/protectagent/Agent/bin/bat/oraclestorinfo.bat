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
rem @date:   2020-04-16
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
set CMD_EXECASMSQL_SIL=execasmsqls

set GETDBINFO="%AGENT_TMP_PATH%GETDBINFO%PID%.sql"
set GETDBINFORST="%AGENT_TMP_PATH%GETDBINFORST%PID%.txt"
set GETASMINFO="%AGENT_TMP_PATH%GETASMINFO%PID%.sql"
set GETASMINFORST="%AGENT_TMP_PATH%GETASMINFORST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclestorinfo.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set FSDevices=
set ASMDevices=
rem #############################################################

rem Get powershell path
call :log "Begin to get powershell path."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1\PowerShellEngine" /v "ApplicationBase"') do set "POWERSHELL_PATH=%%k" 
if "!POWERSHELL_PATH!"=="" (
    call :log "Get powershell path faild."
    set ERR_CODE=%ERR_FILE_IS_EMPTY%
    goto :error
)

call :log "Get powershell path succ, powershell path %POWERSHELL_PATH%."

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

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMINSTANCENAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetDBAuth
call :SetASMAuth

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclestorinfo.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclestorinfo.bat %AGENT_ROOT% %PID%"
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

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;ASMINSTANCENAME=!ASMINSTANCENAME!;ASMUSER=!ASMUSER!"

set DBROLE=
if "%DBUSER%"=="sys" (
    set DBROLE=as sysdba
)

call :DeleteFile %RSTFILE%

call :Log "Begin to get data file capacity"
set /a FILEUSEDCAPACITY=0
set /a FILEALLCAPACITY=0
set DRIVERLIST=
call :GetDBFileCapacity 0
call :Log "Get data file capacity successfully"

call :Log "Begin to get log file capacity"
set /a FILEUSEDCAPACITY=0
set /a FILEALLCAPACITY=0
set DRIVERLIST=
call :GetDBFileCapacity 1
call :Log "Get log file capacity successfully"

call :Log "Begin to get databse type"
call :GetDBStorType
call :Log "Get databse type successfully"

call :Log "Begin to get database instance list"
call :GetDBInstList
call :Log "Get database instance list successfully"

call :DeleteFile %GETASMINFO%
call :DeleteFile %GETASMINFORST%
call :DeleteFile %GETDBINFO%
call :DeleteFile %GETDBINFORST%
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

rem get db storage type
:CreateGetStorageList
    echo set linesize 600 > "%~1"
    echo set pagesize 0 >> "%~1"
    echo set feedback off >> "%~1"
    if "%~2" == "0" (
        echo col name for a513 >> "%~1"
        echo select name from v$datafile; >> "%~1"
    ) else (
        echo col MEMBER for a513 >> "%~1"
        echo select MEMBER from v$logfile; >> "%~1"
    )
    echo exit >> "%~1"
    goto :EOF

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    set %~2=!CONVERTSTR!
goto :EOF

:GetFSCapacity
    set FilePath=%~1
    set WDriver=!FilePath:~0,1!
    if "!WDriver!" == "" (
        call :Log "get driver of path %1 failed."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :error
    )

    rem get file size
    for /f "delims=" %%i in ("!FilePath!") do (
        set /a filesize=%%~zi / 1048576
    )

    set /a FILEUSEDCAPACITY=!FILEUSEDCAPACITY! + !filesize!

    rem get driver size
    echo !DRIVERLIST! | findstr ":!WDriver!:" >nul
    rem only get total capacity when device isn't count
    if !errorlevel! NEQ 0 ( 
        call "%POWERSHELL_PATH%\powershell.exe" -command "gwmi win32_volume | where {$_.DriveLetter -eq '!WDriver!:' } | select-object @{Name='Capacity';Expression={$_.Capacity/1MB}} | fl" > %CURRENTPIDRST%
        set /a driverSize=0
        for /f "tokens=1,2 delims=.: " %%a in ('type %CURRENTPIDRST%') do (
            if "%%a" == "Capacity" (
                set /a driverSize=%%b
            )
        )
        call :DeleteFile %CURRENTPIDRST%
        set /a FILEALLCAPACITY=!FILEALLCAPACITY! + !driverSize!
        set DRIVERLIST=!DRIVERLIST!:!WDriver!:
    )
goto :EOF

:GetASMCapacity
    set FilePath=%~1
    set DGNAME=
    for /f "tokens=1,2 delims=+/" %%a in ("!FilePath!") do (
        set DGNAME=%%a
    )

    echo !DRIVERLIST! | findstr ":!DGNAME!:" >nul
    rem only get total capacity when device isn't count
    if !errorlevel! NEQ 0 (
        call :Log "query diskgroup !DGNAME! capacity."
        echo set pagesize 0; > "%GETASMINFO%"
        echo set feedback off; >> "%GETASMINFO%"
        echo select TOTAL_MB,FREE_MB from v$asm_diskgroup where Name='!DGNAME!'; >> "%GETASMINFO%"
        echo exit >> "%GETASMINFO%"

        call :Log "Exec sql to get asm diskgroup !DGNAME! capacity."
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %GETASMINFO% %GETASMINFORST% %ASMINSTANCENAME% "!ASM_LOGIN!" 60 RetCode
        if !RetCode! NEQ 0 (
            set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
            goto :error
        ) else (
            rem get disk list
            for /f "tokens=1,2 delims= " %%a in ('type %GETASMINFORST%') do (
                set /a FILEALLCAPACITY=!FILEALLCAPACITY! + %%a
                set /a FILEUSEDCAPACITY=!FILEUSEDCAPACITY! + %%b
                set DRIVERLIST=!DRIVERLIST!:!DGName!:
                goto :EOF
            )
        )
    )
goto :EOF

:GetDBFileCapacity
    call :CreateGetStorageList %GETDBINFO% "%~1"
    call :Log "Exec sql to get storage list of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %GETDBINFO% %GETDBINFORST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Execute Script to query DB %DBNAME% file list failed."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :error
    ) else (
        for /f "skip=1 delims=" %%i in ('type %GETDBINFORST%') do (
            set LINE=%%i
            if not !LINE! == "" (
                set FSTCHAR=!LINE:~0,1!
                if "!FSTCHAR!" == "+" (
                    call :GetASMCapacity "!LINE!"
                ) else (
                    call :GetFSCapacity "!LINE!"
                )
            )
        )

        if "%~1" == "0" (
            call :Log "datacap;!FILEUSEDCAPACITY!;!FILEALLCAPACITY!"
            echo datacap;!FILEUSEDCAPACITY!;!FILEALLCAPACITY!>> %RSTFILE%
        ) else (
            call :Log "logcap;!FILEUSEDCAPACITY!;!FILEALLCAPACITY!"
            echo logcap;!FILEUSEDCAPACITY!;!FILEALLCAPACITY!>> %RSTFILE%
        )
    )

    goto :eof

rem get db storage type
:CreateGetStorageType
    echo set linesize 1000 > "%~1"
    echo set heading off >> "%~1"
    echo set pagesize 0 >> "%~1"
    echo set feedback off >> "%~1"
    echo select name from v$datafile; >> "%~1"
    echo select MEMBER from v$logfile; >> "%~1"
    echo select name from v$controlfile; >> "%~1"
    echo select VALUE from v$parameter where name='spfile'; >> "%~1"
    echo exit >> "%~1"
    goto :EOF

:GetDBStorType
    call :CreateGetStorageType %GETDBINFO%
    call :Log "Exec sql to get storage type of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %GETDBINFO% %GETDBINFORST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Execute Script to query DB %DBINSTANCE% file list failed."
        call :DeleteFile %GETDBINFO%
        call :DeleteFile %GETDBINFORST%
        exit !RetCode!
    ) else (
        set /a ISASM=1
        for /f %%i in ('type %GETDBINFORST%') do (
            set LINE=%%i
            set FSTCHAR=!LINE:~0,1!
            if "!FSTCHAR!" == "+" (
                call :Log "database type ASM"
                set /a ISASM=0
            )
        )
        echo dbtype;!ISASM!>> %RSTFILE%
        call :Log "dbtype;!ISASM!"
        call :DeleteFile %GETDBINFO%
        call :DeleteFile %GETDBINFORST%
        goto :eof
    )
    goto :eof

:GetDBInstList
    echo set linesize 20 > %GETDBINFO%
    echo set heading off >> %GETDBINFO%
    echo set pagesize 0 >> %GETDBINFO%
    echo set feedback off >> %GETDBINFO%
    echo select instance_name from gv$instance; >> %GETDBINFO%
    echo exit >> %GETDBINFO%
    call :Log "Exec sql to get instance list of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %GETDBINFO% %GETDBINFORST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Execute Script to query DB %DBINSTANCE% instacne list failed."
        call :DeleteFile %GETDBINFO%
        call :DeleteFile %GETDBINFORST%
        exit !RetCode!
    ) else (
        set dbList=
        for /f %%i in ('type %GETDBINFORST%') do (
            if "!dbList!" == "" (
                set dbList=%%i
            ) else (
                set dbList=!dbList!;%%i
            )
        )
        echo dbInsLst;!dbList!>> %RSTFILE%
        call :Log "dbInsLst;!dbList!"
        call :DeleteFile %GETDBINFO%
        call :DeleteFile %GETDBINFORST%
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

:error
    call :DeleteFile %GETASMINFO%
    call :DeleteFile %GETASMINFORST%
    call :DeleteFile %GETDBINFO%
    call :DeleteFile %GETDBINFORST%
    exit /b %ERR_CODE%
    
:end
endlocal
