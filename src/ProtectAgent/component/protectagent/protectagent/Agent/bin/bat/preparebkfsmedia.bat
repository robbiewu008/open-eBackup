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
rem @date:   2020-04-24
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

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set LOGFILE=preparebkfsmedia.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
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
    goto :end
)

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "MOUNTPATH" MOUNTPATH
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "DISKLIST" DISKLIST
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "FSTYPE" FSTYPE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "CREATEMEDIA" CREATEMEDIA
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "HOSTROLE" HOSTROLE
call :Log "MOUNTPATH=%MOUNTPATH%;DISKLIST=%DISKLIST%;FSTYPE=%FSTYPE%;CREATEMEDIA=%CREATEMEDIA%;HOSTROLE=%HOSTROLE%"

set fstChar=%MOUNTPATH:~0,1%
if not "%fstChar%" == "+" (
    if not exist "%MOUNTPATH%" (
        md "%MOUNTPATH%" 
    )
)

call :Log "Begin to prepare fs backup medium."
set ERR_CODE=0
call :GetCodedFormat

rem only use the 1st disk
for /f "tokens=1 delims=," %%a in ("!DISKLIST!") do (
    set diskInfo=%%a
    for /f "tokens=1,2 delims=-" %%b in ("!diskInfo!") do (
        set diskNum=%%b
        set isExtend=%%c
        call :CreateFSMeidum "!diskNum!" "!isExtend!"
    )
)
call :Log "Finish preparing fs backup medium."
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
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )

    set %~2=!CONVERTSTR!
goto :EOF

:CreateFSMeidum
    set diskNum=%~1
    set isExtend=%~2
    rem 1.online disk
    call :OnlineDisk !diskNum!

    rem sleep 1 second agaist the situation, disk is offline when diskpart opertion
    call :WinSleep 1

    rem 2.check need to create medium, restore do't create medium
    if "!CREATEMEDIA!" == "1" (
        set /a pFlag=0
        chcp 65001
        echo select disk !diskNum! > %DISKPART_CMD_FILE%
        echo list partition >> %DISKPART_CMD_FILE%
        for /f %%a in ('diskpart /s %DISKPART_CMD_FILE% ^| findstr "Primary"') do (
            set /a pFlag=1
        )
        chcp !oldCodedFormat!
        if !pFlag! EQU 0 (
            rem there is no partition, need to create
            echo select disk !diskNum! > %DISKPART_CMD_FILE%
            echo CONVERT GPT >> %DISKPART_CMD_FILE%
            echo create partition primary >> %DISKPART_CMD_FILE%
            echo format FS=NTFS quick >> %DISKPART_CMD_FILE%
            echo remove all DISMOUNT NOERR >> %DISKPART_CMD_FILE%
            echo assign mount="%MOUNTPATH%" >> %DISKPART_CMD_FILE%
            diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
            if !errorlevel! NEQ 0 (
                call :Log "create partition failed"
                set ERR_CODE=1
                goto :end
            ) else (
                call :Log "create backup medium successfully."
                goto :eof
            )
        )
    )

    set partNum=
    rem get partition
    chcp 65001
    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo list partition >> %DISKPART_CMD_FILE%
    rem get the primary and extend volume
    for /f "tokens=2 delims= " %%a in ('diskpart /s %DISKPART_CMD_FILE% ^| findstr "Primary"') do (
        set partNum=%%a
    )
    chcp !oldCodedFormat!
    if "!partNum!" == "" (
        call :Log "Get partition number failed."
        set ERR_CODE=1
        goto :end
    )
    call :Log "partNum=!partNum!"

    rem 3.check whether to extend volume
    if "!isExtend!" == "1" (
        echo select disk !diskNum! > %DISKPART_CMD_FILE%
        echo select partition !partNum! >> %DISKPART_CMD_FILE%
        echo extend >> %DISKPART_CMD_FILE%
        echo remove all DISMOUNT NOERR >> %DISKPART_CMD_FILE%
        echo assign mount="%MOUNTPATH%" >> %DISKPART_CMD_FILE%
        diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
        if !errorlevel! NEQ 0 (
            call :Log "extend partition failed"
            set ERR_CODE=1
            goto :end
        ) else (
            call :Log "extend backup medium successfully."
            goto :eof
        )
    )

    rem 4.assign mount path
    echo select disk !diskNum! > %DISKPART_CMD_FILE%
    echo select partition !partNum! >> %DISKPART_CMD_FILE%
    echo remove all DISMOUNT NOERR >> %DISKPART_CMD_FILE%
    echo assign mount="%MOUNTPATH%" >> %DISKPART_CMD_FILE%
    diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
    if !errorlevel! NEQ 0 (
        call :Log "assign partition mount path failed"
        set ERR_CODE=1
        goto :end
    ) else (
        call :Log "assign partition mount path successfully."
    )

goto :EOF

:OnlineDisk
    echo select disk %~1 > %DISKPART_CMD_FILE%
    echo att disk clear readonly noerr>> %DISKPART_CMD_FILE%
    echo online disk >> %DISKPART_CMD_FILE%
    diskpart /s %DISKPART_CMD_FILE% >> %LOGFILEPATH%
    call :Log "online disk %~1 successfully."
goto :EOF

:GetCodedFormat
    for /f "tokens=2 delims=:" %%a in ('chcp') do (
        set oldCodedFormat=%%a
    )
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof

:end
    call :DeleteFile %DISKPART_CMD_FILE%
    exit !ERR_CODE!

endlocal
