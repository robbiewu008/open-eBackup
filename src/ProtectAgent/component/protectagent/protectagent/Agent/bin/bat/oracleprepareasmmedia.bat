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
rem @date:   2020-04-26
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

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"

set LOGFILE=oracleprepareasmmedia.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set DISKPART_CMD_FILE="%AGENT_TMP_PATH%diskCmd%PID%"

rem execute sql
set ASMSQL="%AGENT_TMP_PATH%asmsql%PID%.sql"
set ASMSQLRST="%AGENT_TMP_PATH%asmsqlRst%PID%.txt"
rem #############################################################

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
            more %CURRENTCMDLineRST% | findstr /c:"oracleprepareasmmedia.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oracleprepareasmmedia.bat %AGENT_ROOT% %PID%"
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
    goto :end
)

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "DGNAME" DGNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMInstanceName
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUserName
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMPassword
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "DISKLIST" DISKLIST
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "CREATEMEDIA" CREATEMEDIA
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "METADATAPATH" METADATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "HOSTROLE" HOSTROLE

call :Log "DGNAME=%DGNAME%;ASMInstanceName=%ASMInstanceName%;ASMUserName=%ASMUserName%;DISKLIST=%DISKLIST%;CREATEMEDIA=%CREATEMEDIA%;METADATAPATH=%METADATAPATH%;CURRENTPID=!CURRENTPID!;HOSTROLE=%HOSTROLE%"

if "%DGNAME%" == "" (
    call :Log "DGNAME is null."
    goto :end
)

if "%ASMInstanceName%" == "" (
    call :Log "ASMInstanceName is null."
    goto :end
)

if "%DISKLIST%" == "" (
    call :Log "DISKLIST is null."
    goto :end
)

if not exist "%METADATAPATH%" (
    md "%METADATAPATH%" 
)

call :Log "Begin to prepare asm backup medium."
set ERR_CODE=0
set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetASMAuth

rem 1.check dg exists, 1:exist 0:no exist
set dgExist=0
call :CheckDGExists %DGNAME% dgExist
if "!dgExist!" == "0" (
    rem diskgroup is not exist, need to create dg
    call :CreateDG
)

rem diskgroup exist, need to extend dg
call :ExtendDG

call :Log "Finish preparing asm backup medium."
goto :end

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
:ExtendDG
    set EXTEND_ASM_DISKS=
    for /f "tokens=1 delims=," %%a in ("!DISKLIST!") do (
        set diskInfo=%%a
        for /f "tokens=1,2,3 delims=-" %%b in ("!diskInfo!") do (
            set diskNum=%%b
            set isExtend=%%c
            set isMeta=%%d
            call :ExtendASMDisk "!diskNum!" "!isExtend!" "!isMeta!"
        )
    )

    if not "!EXTEND_ASM_DISKS!" == "" (
        rem extend diskgroup
        echo alter diskgroup !dgName! add disk !EXTEND_ASM_DISKS!; > %ASMSQL%
        echo exit >> %ASMSQL%
        call :Log "Begin to extend diskgroup !dgName! with !EXTEND_ASM_DISKS!."
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %ASMSQL% %ASMSQLRST% %ASMInstanceName% "!ASM_LOGIN!" 60 RetCode
        if !RetCode! NEQ 0 (
            call :Log "extend diskgroup !dgName! failed."
            set ERR_CODE=!RetCode!
            goto :end
        )
    )
goto :EOF

:ExtendASMDisk
    set diskNum=%~1
    set isExtend=%~2
    set isMeta=%~3

    if "!isExtend!" == "0" (
        goto :EOF
    )

    rem 2.gte free space
    call :log "Begin to get powershell path."
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1\PowerShellEngine" /v "ApplicationBase"') do set "POWERSHELL_PATH=%%k" 
    if "!POWERSHELL_PATH!"=="" (
        call :log "Get powershell path faild."
        set ERR_CODE=%ERR_FILE_IS_EMPTY%
        goto :end
    )
    call :log "Get powershell path succ, powershell path %POWERSHELL_PATH%."

    call "%POWERSHELL_PATH%\powershell.exe" -command "Get-Disk !diskNum! | select-object @{Name='LargestFreeExtent';Expression={$_.LargestFreeExtent/1GB}} | fl" > %DISKPART_CMD_FILE%
    set /a freeSize=0
    for /f "tokens=2 delims=.: " %%a in ('type %DISKPART_CMD_FILE%') do (
        set /a freeSize=%%a
    )

    rem 3.check disk free space
    if !freeSize! LEQ 0 (
        set ERR_CODE=1
        call :Log "disk !diskNum! have no free size"
        type %DISKPART_CMD_FILE% >> %LOGFILEPATH%
        goto :end
    )

    rem 4.new volume for asm
    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo create partition primary >> %DISKPART_CMD_FILE%
    diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
    if !errorlevel! NEQ 0 (
        call :Log "create partition with free space failed"
        set ERR_CODE=1
        goto :end
    )

    rem 5.get lastest partition number
    set partNum=
    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo list partition >> %DISKPART_CMD_FILE%
    for /f "tokens=2 delims= " %%a in ('diskpart /s %DISKPART_CMD_FILE% ^| findstr "Primary"') do (
        set partNum=%%a
    )
    if "!partNum!" == "" (
        call :Log "Get extend new partition number failed."
        set ERR_CODE=1
        goto :end
    )
    call :Log "diskNun=!diskNum!;partNum=!partNum!"

    rem 6.label asm disk
    asmtool -addprefix asm \Device\Harddisk!diskNum!\Partition!partNum! >> %LOGFILEPATH%
    if !errorlevel! NEQ 0 (
        call :Log "label asm disk for disk=!diskNum! partition=!partNum! failed."
        set ERR_CODE=1
        goto :end
    )

    rem get disk name
    set ASM_DISK=
    for /f "tokens=1 delims= " %%a in ('asmtool -list ^| findstr "\Device\Harddisk!diskNum!\Partition!partNum!"') do (
        set ASM_DISK=%%a
    )
    if "!ASM_DISK!" == "" (
        call :Log "Get partition asm disk failed, partition=\Device\Harddisk!diskNum!\Partition!partNum!."
        set ERR_CODE=1
        goto :end
    )

    if "!EXTEND_ASM_DISKS!" == "" (
        set EXTEND_ASM_DISKS='\\.\!ASM_DISK!'
    ) else (
        set EXTEND_ASM_DISKS=!CREATE_ASM_DISKS!,'\\.\!ASM_DISK!'
    )

    call :Log "extend asm disk !ASM_DISK! with \Device\Harddisk!diskNum!\Partition!partNum! successfully."
goto :EOF

:CreateDG
    set CREATE_ASM_DISKS=
    for /f "tokens=1 delims=," %%a in ("!DISKLIST!") do (
        set diskInfo=%%a
        for /f "tokens=1,2,3 delims=-" %%b in ("!diskInfo!") do (
            set diskNum=%%b
            set isExtend=%%c
            set isMeta=%%d
            call :CreateASMDisk "!diskNum!" "!isExtend!" "!isMeta!"
        )
    )

    if not "!CREATE_ASM_DISKS!" == "" (
        rem create diskgroup
        echo create diskgroup !dgName! external redundancy disk !CREATE_ASM_DISKS!; > %ASMSQL%
        echo exit >> %ASMSQL%
        call :Log "Begin to create diskgroup !dgName! with !CREATE_ASM_DISKS!."
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %ASMSQL% %ASMSQLRST% %ASMInstanceName% "!ASM_LOGIN!" 60 RetCode
        if !RetCode! NEQ 0 (
            call :Log "create diskgroup !dgName! failed."
            set ERR_CODE=!RetCode!
            goto :end
        )
    )

    rem mount diskgroup
    echo select state from v$asm_diskgroup where name='!dgName!'; > %ASMSQL%
    echo exit >> %ASMSQL%
    call :Log "Begin to check diskgroup !dgName! state."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %ASMSQL% %ASMSQLRST% %ASMInstanceName% "!ASM_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "check diskgroup !dgName! state failed."
        set ERR_CODE=!RetCode!
        goto :end
    )

    rem get diskgroup state
    set dgState=
    for /f %%a in ('type %ASMSQLRST%') do (
        set dgState=%%a
    )

    if "!dgState!" == "" (
        call :Log "there is no diskgroup !dgName!"
        goto :end
    )

    call :Log "diskgroup !dgName! state=!dgState!"
    if not "!dgState!" == "MOUNTED" (
        rem mount diskgroup
        echo alter diskgroup !dgName! mount; > %ASMSQL%
        echo exit >> %ASMSQL%
        call :Log "Begin to mount diskgroup !dgName!."
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %ASMSQL% %ASMSQLRST% %ASMInstanceName% "!ASM_LOGIN!" 60 RetCode
        if !RetCode! NEQ 0 (
            call :Log "mount diskgroup !dgName! failed."
            set ERR_CODE=!RetCode!
            goto :end
        )
    )
goto :EOF

:CreateASMDisk
    set diskNum=%~1
    set isExtend=%~2
    set isMeta=%~3

    rem 1.online disk
    call :OnlineDisk !diskNum!
	rem sleep 1 second agaist the situation, disk is offline when diskpart opertion
	call :WinSleep 1
	
    if !isExtend! == "1" (
        goto :EOF
    )

    rem 2.check partition list
	set /a pFlag=0
    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo list partition >> %DISKPART_CMD_FILE%
	for /f %%a in ('diskpart /s %DISKPART_CMD_FILE% ^| findstr "Primary"') do (
		set /a pFlag=1
	)
    if !pFlag! EQU 0 (
        rem only create in backup progress
        if "!CREATEMEDIA!" == "0" (
            call :Log "CREATEMEDIA is 0, need to create medium, but this is backup task, shouldn't create new one."
            set ERR_CODE=1
            goto :end
        )

        rem there is no partition, need to create
        echo select disk !diskNum! > %DISKPART_CMD_FILE%
        echo CONVERT GPT >> %DISKPART_CMD_FILE%
        if "!isMeta!" == "1" (
            echo create partition primary size=1024 >> %DISKPART_CMD_FILE%
            echo format FS=NTFS quick >> %DISKPART_CMD_FILE%
            echo assign mount="%METADATAPATH%" >> %DISKPART_CMD_FILE%
        )
        echo create partition primary >> %DISKPART_CMD_FILE%
        diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
		set /a rst=!errorlevel!
        if !rst! NEQ 0 (
            call :Log "create partition failed, rst=!rst!."
            set ERR_CODE=1
            goto :end
        )

        set partNum=
        rem get partition number
        echo select disk !diskNum! > %DISKPART_CMD_FILE%
        echo list partition >> %DISKPART_CMD_FILE%
        rem get the last volume for label asm
        for /f "tokens=2 delims= " %%a in ('diskpart /s %DISKPART_CMD_FILE% ^| findstr "Primary"') do (
            set partNum=%%a
        )
        if "!partNum!" == "" (
            call :Log "Get partition number failed."
            set ERR_CODE=1
            goto :end
        )
        call :Log "partNum=!partNum!"

        rem label asm disk
        asmtool -addprefix asm \Device\Harddisk!diskNum!\Partition!partNum! >> %LOGFILEPATH%
        if !errorlevel! NEQ 0 (
            call :Log "label asm disk for disk=!diskNum! partition=!partNum! failed."
            set ERR_CODE=1
            goto :end
        )

        rem get disk name
        set ASM_DISK=
        for /f "tokens=1 delims= " %%a in ('asmtool -list ^| findstr "\Device\Harddisk!diskNum!\Partition!partNum!"') do (
            set ASM_DISK=%%a
        )
        if "!ASM_DISK!" == "" (
            call :Log "Get partition asm disk failed, partition=\Device\Harddisk!diskNum!\Partition!partNum!."
            set ERR_CODE=1
            goto :end
        )

        if "!CREATE_ASM_DISKS!" == "" (
            set CREATE_ASM_DISKS='\\.\!ASM_DISK!'
        ) else (
            set CREATE_ASM_DISKS=!CREATE_ASM_DISKS!,'\\.\!ASM_DISK!'
        )

        call :Log "create asm disk !ASM_DISK! with \Device\Harddisk!diskNum!\Partition!partNum! successfully."
    )
goto :EOF

:CheckDGExists
    set dgName=%~1
    echo set feedback off > %ASMSQL%
    echo set heading off >> %ASMSQL%
    echo set pagesize 0 >> %ASMSQL%
    echo select count(*) from v$asm_diskgroup where name='!dgName!' and state='MOUNTED'; >> %ASMSQL%
    echo exit >> %ASMSQL%

    call :Log "Begin to check diskgroup !dgName! number."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %ASMSQL% %ASMSQLRST% %ASMInstanceName% "!ASM_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Check diskgroup !dgName! number failed."
        set ERR_CODE=!RetCode!
        goto :end
    ) else (
        rem get diskgroup number
        set /a dgNum=0
        for /f %%a in ('type %ASMSQLRST%') do (
            set /a dgNum=%%a
        )

        if !dgNum! GTR 0 (
            set %~2=1
        ) else (
            set %~2=0
        )
    )
    call :Log "Finish checking diskgroup !dgName! number=!dgNum!."
goto :EOF

:OnlineDisk
	echo select disk %~1 > %DISKPART_CMD_FILE%
	echo att disk clear readonly noerr>> %DISKPART_CMD_FILE%
	echo online disk >> %DISKPART_CMD_FILE%
	diskpart /s %DISKPART_CMD_FILE%
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof

:SetASMAuth
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=%ASMUserName%/"%ASMPassword%" as sysasm
    )
goto :eof

:end
    call :DeleteFile %DISKPART_CMD_FILE%
    call :DeleteFile %ASMSQL%
    call :DeleteFile %ASMSQLRST%
    exit !ERR_CODE!

endlocal
