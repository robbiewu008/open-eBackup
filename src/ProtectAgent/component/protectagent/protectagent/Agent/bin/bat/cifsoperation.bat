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
rem @dest:   application agent for cifs
rem @date:   2022-06-18
rem @authr:
rem @modify:

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%agent_com.bat"
set CMD_GETVALUE=getvalue

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=cifsoperation.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
rem #############################################################
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "JobID" JobID
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "SubPath" SubPath
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "OperationType" OperationType
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "StorageIp" StorageIp
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "FilesystemName" FilesystemName
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "AuthKey" AuthKey
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "MountDriver" MountDriver
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "LinkPath" LinkPath
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "SharedPath" SharedPath
call :DeleteFile !PARAM_FILE!
call :GetCURRENTPID
call :DeleteFile !CURRENTPIDRST!
call :DeleteFile !CURRENTCMDLineRST!
call :Log "JobID=%JobID%;OperationType=%OperationType%;LinkPath=%LinkPath%;SubPath=%SubPath%;StorageIp=%StorageIp%;FilesystemName=%FilesystemName%;AuthKey=%AuthKey%;MountDriver=%MountDriver%;SharedPath=%SharedPath%"

if "!OperationType!" == "mount" (
    call :Mount
) else if "!OperationType!" == "unmount" (
    call :Unmount
) else (
	call :Log "error OperationType=!OperationType!."
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
)
goto :end


:Log
    echo %date:~0,10% %time:~0,8% [%username%] [!CURRENTPID!] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
goto :EOF

:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
goto :EOF

:GetCURRENTPID
	set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
	set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
	set /a CURRENTPID=0
	set /a NUM=0
	call wmic process where name="cmd.exe" get processid > %CURRENTPIDRST%
	for /f %%a in ('type %CURRENTPIDRST%') do (
		if !NUM! NEQ 0 (
			set processID=%%a
			call wmic process where processid="!processID!" get Commandline > %CURRENTCMDLineRST%
			set AGENT_ROOT_TMP=%AGENT_ROOT: =%
			if "!AGENT_ROOT_TMP!" == "!AGENT_ROOT!" (
				more %CURRENTCMDLineRST% | findstr /c:"cifsoperation.bat %AGENT_ROOT% %PID%" 1>nul 2>&1
			) else (
				more %CURRENTCMDLineRST% | findstr /c:"cifsoperation.bat\" \"%AGENT_ROOT%\" %PID%" 1>nul 2>&1
			)
			if !errorlevel! EQU 0 (
				set /a CURRENTPID=%%a
				goto :EOF
			)
		)
		set /a NUM=!NUM!+1
	)
goto :EOF

:AfterMountSuccess
	set StorageIp=%1
	set SharedName=%2
	call :Log "StorageIp=!StorageIp!, SharedName=!SharedName!."

	if not "!SubPath!" == "" (
		set SubPath=!SubPath::= !
		for %%a in (!SubPath!) do (
			if not exist !MountDriver!\%%a (
			    md \\!StorageIp!\!SharedName!\%%a
			    call :Log "md \\!StorageIp!\!SharedName!\%%a."
			)
		)
	)
	md !LinkPath!
	mklink /d !LinkPath!\!StorageIp! \\!StorageIp!\!SharedName!
	if !errorlevel! NEQ 0 (
        call :Log "mklink \\!StorageIp!\!SharedName! on !LinkPath!\!StorageIp! failed."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :end
	)
	echo !LinkPath!\!StorageIp!> !RSTFILE!
goto :EOF

:CheckMount
    rem 当FilesystemName中有\时，需转化为\\，否则findstr将会失败
    set FsMatch=%FilesystemName:\=\\%
	@REM 重启主机后，共享连接断开，但挂载链接目录还在，需删除挂载链接目录后重新连接
    net use | findstr \\\\%StorageIp%\\%FsMatch% 1>nul
    if !errorlevel! NEQ 0 (
		call :Log "Shared \\%StorageIp%\%FilesystemName% is not exist."
        if exist !LinkPath!\!StorageIp! (
            call :Log "Link path !LinkPath!\!StorageIp! is exist, will remove it."
            rmdir !LinkPath!\!StorageIp! >> %LOGFILEPATH% 2>&1
        )
		goto :CheckEnd
    )

	@REM 共享已连接，但状态为Disconnected，删除后重新连接
    net use | findstr \\\\%StorageIp%\\%FsMatch% | findstr Disconnected 1>nul
    if !errorlevel! EQU 0 (
		call :Log "Shared \\%StorageIp%\%FilesystemName% is exist, but status is disconnected, will remove it."
        net use \\%StorageIp%\%FilesystemName% /del /y >> %LOGFILEPATH% 2>&1
		goto :CheckEnd
    )
	if exist !LinkPath!\!StorageIp! (
        call :Log "\\!StorageIp!\!FilesystemName! is already mount on !LinkPath!\!StorageIp!."
		echo !LinkPath!\!StorageIp!> !RSTFILE!
        goto :end
    )
	:CheckEnd
goto :EOF

:Mount
    set MountDriver=
    call :CheckMount

	for /f "tokens=2 delims=:" %%a in ('chcp') do (
        set oldCodedFormat=%%a
    )
	chcp 65001
	for /l %%i in (1,1,3) do (
        call !AGENT_BIN_PATH!\agentcli.exe MountCifs "net use \\!StorageIp!\!FilesystemName! \"*\" /user:localdomain\!AuthKey! /PERSISTENT:YES" >> %LOGFILEPATH% 2>&1
        if "!errorlevel!"=="0" (
            goto :loopEnd
        )
        call :WinSleep 5
    )
    :loopEnd
    chcp !oldCodedFormat!

    rem 当FilesystemName中有\时，需转化为\\，否则findstr将会失败
    set FsMatch=%FilesystemName:\=\\%
    if "%FilesystemName%" == "" (
		rem 老版本逻辑中存在FilesystemName为空，但判断挂载成功的场景，为减少影响，此块场景维持不变
        set FsMatch=%FilesystemName%
    )
    net use | findstr \\\\%StorageIp%\\%FsMatch% | findstr OK 1>nul
    if not "!errorlevel!"=="0" (
        call :Log "Mount failed, \\!StorageIp!\!FilesystemName! is not exist."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :end
    )
	call :Log "\\!StorageIp!\!FilesystemName! mount success."
	call :AfterMountSuccess !StorageIp! !FilesystemName!
goto :EOF

:Unmount
    net use !SharedPath! /del /y >> %LOGFILEPATH% 2>&1
    if !errorlevel! NEQ 0 (
		if exist !LinkPath! (
			rd /s /q !LinkPath!
		)
        call :Log "Unmount failed, !SharedPath! is still exist."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :end
    )
	if exist !LinkPath! (
		rd /s /q !LinkPath!
	)
	call :Log "Unmount !SharedPath! success."
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof

:end
    call :Log "Finish !OperationType! cifs filesystem."
    exit !ERR_CODE!

endlocal
