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
set CMD_EXECASMSQL_SIL=execasmsqls

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclenativedismount.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryServiceRST%PID%.txt"
set DISKPART_CMD_FILE="%AGENT_TMP_PATH%diskCmd%PID%"
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

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMSIDNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "DISKLIST" DISKLIST

if "!DISKLIST!" == "" (
    call :Log "DISKLIST is null"
    set ERR_CODE=1
    goto :error
)

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativedismount.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativedismount.bat %AGENT_ROOT% %PID%"
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
call :Log "ASMSIDNAME=%ASMSIDNAME%;ASMUSER=%ASMUSER%;CURRENTPID=%CURRENTPID%;DISKLIST=!DISKLIST!"

if not "!ASMSIDNAME!" == "" (
    sc query state= all | findstr /i "OracleASMService%ASMSIDNAME%" > %QUREYSERVICE%
    for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
        if "SERVICE_NAME"=="%%a" (
            for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
                set ASMSIDNAME=+%%j
            )
        )
    )
    call :DeleteFile %QUREYSERVICE%
    call :Log "check ASM Instance=!ASMSIDNAME!"
    
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "PATHDATA" DGDATA
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "PATHLOG" DGLOG
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "METADATAPATH" METADATAPATH

    set AUTHMODE=0
    if "%ASMUSERPWD%" == "" (
        set AUTHMODE=1
    )
    call :SetASMAuth

    call :Log "AUTHMODE=!AUTHMODE!;DGDATA=!DGDATA!;DGLOG=!DGLOG!;METADATAPATH=!METADATAPATH!"
    if "!DGDATA!" == "" (
        call :Log "DGDATA is null"
        set ERR_CODE=1
        goto :error
    )
    if "!DGLOG!" == "" (
        call :Log "DGLOG is null"
        set ERR_CODE=1
        goto :error
    )

    rem dismount diskgroup
    echo alter diskgroup !DGDATA! dismount; > %CHECKINSTANCESTATUS%
    echo alter diskgroup !DGLOG! dismount; >> %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%
    call :Log "Begin to dismount diskgroup !DGDATA!,!DGLOG!"
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% !ASMSIDNAME! "!ASM_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "dismount diskgroup !DGDATA!,!DGLOG! failed"
        set ERR_CODE=!RetCode!
        goto :error
    )
    call :Log "Dismount diskgroup !DGDATA!,!DGLOG! successfully"
)

rem offline disk
rem fs only offline disk
rem asm dismount diskgroup and offline disk, now support 2 disks only
rem TODO, to support more disk number, must use powershell
for /f "tokens=1,2 delims=," %%a in ("!DISKLIST!") do (
    if not "%%a" == "" (
        call :OfflineDisk "%%a"
    )
    if not "%%b" == "" (
        call :OfflineDisk "%%b"
    )
)

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

:SetASMAuth
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=%ASMUSER%/"%ASMUSERPWD%" as sysasm
    )
goto :eof

:OfflineDisk
    set diskNum=
    for /f "tokens=1 delims=-" %%b in ("%~1") do (
        set diskNum=%%b
        goto :BREAKFOR
    )

:BREAKFOR
    call :Log "diskNum=!diskNum!"

    set partNum=
    rem get partition
    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo list partition >> %DISKPART_CMD_FILE%
    rem get the primary and extend volume
    for /f "tokens=2 delims= " %%a in ('diskpart /s %DISKPART_CMD_FILE% ^| findstr "Primary"') do (
        set partNum=%%a
    )
    if "!partNum!" == "" (
        call :Log "Get partition number failed."
        set ERR_CODE=1
        goto :end
    )
    call :Log "partNum=!partNum!"

    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo select partition !partNum! >> %DISKPART_CMD_FILE%
    echo remove all DISMOUNT NOERR >> %DISKPART_CMD_FILE%
    echo offline disk >> %DISKPART_CMD_FILE%
    diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
    if !errorlevel! EQU 0 (
        call :Log "offline disk !diskNum! successfully"
    ) else (
        call :Log "offline disk !diskNum! failed"
        set ERR_CODE=1
        goto :error
    )
goto :eof

:error
    call :DeleteFile %CHECKINSTANCESTATUS%
    call :DeleteFile %CHECKINSTANCESTATUSRST%
    call :DeleteFile %DISKPART_CMD_FILE%
    exit /b %ERR_CODE%
endlocal
