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
setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a RET_CODE=0

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set MOUNT_HOME_PATH=C:\dataturbo_mnt
set COMMONFUNC="%AGENT_BIN_PATH%agent_com.bat"
set CMD_GETVALUE=getvalue

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=dataturbo_mount.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
rem #############################################################
call :LogInfo "********************************Start to execute the dataturbo mount script********************************"
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "storageName" ParamStorageName
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "mountPath" ParamMountPath
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "nasFileSystemName" ParamNasFileSystemName
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "repositoryType" ParamRepositoryType
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "runAccount" ParamRunAccount
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "subPath" ParamSubPath
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "uid" ParamUid
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "gid" ParamGid
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% !PARAM_FILE! "mode" ParamMode

call :DeleteFile !PARAM_FILE!
call :GetCURRENTPID
call :DeleteFile !CURRENTPIDRST!
call :DeleteFile !CURRENTCMDLineRST!

set ParamMountPath=%ParamMountPath:/=\%

call :LogInfo "PID=!PID!;MountPath=!ParamMountPath!;NasFilesystem=!ParamNasFileSystemName!;StorageName=!ParamStorageName!;RepositoryType=!ParamRepositoryType!;UserID=!ParamUid!;GroupID=!ParamGid!;Mode=!ParamMode!;SubPath=!ParamSubPath!."


set needReExec=0
set rootFs=
set prefixHost=
set patchStr=
echo !ParamMountPath! | findstr /c:"!ParamNasFileSystemName!" >nul
if "!errorlevel!" == "0" (
    set needReExec=0
)
if not exist !ParamMountPath! (
    mkdir !ParamMountPath!
    if not "!errorlevel!" == "0" (
        call :LogInfo "create folder !ParamMountPath! failed."
        set RET_CODE=!ERROR_SCRIPT_EXEC_FAILED!
        goto :end
    )
    call :LogInfo "The dataturbo hostMount directory !ParamMountPath! has been created."
)

call :CheckMountPath
if "!errorlevel!" == "0" (
    call :LogInfo "Mounted !ParamMountPath!, will skip mount operation."
    goto :end
)
call :MountDataturboFs
if "!ParamRepositoryType!" == "log" (
    call :CreateFile "!ParamMountPath!\.agentlastlogbackup.meta"
    call :CreateFile "!ParamMountPath!\.dmelastlogbackup.meta"

    for /f "tokens=1,2 delims=:" %%a in ('echo !ParamSubPath!') do (
        set tmpPath=!ParamMountPath!\%%a
        if not exist !tmpPath! (
            mkdir !tmpPath!
        )
        set tmpPath=!ParamMountPath!\%%b
        if not exist !tmpPath! (
            mkdir !tmpPath!
        )
    )
)
goto :end

:CreateFile
    set filePath=%~1
    set retryTime=3
    if not exist !filePath! (
        echo.> !filePath!
    )
    for /l %%a in (1 1 !retryTime!) do (
        if not exist !filePath! (
            call :LogErr "Create file !filePath! failed retry %%a time."
            call :WinSleep 3
            echo.> !filePath!
        ) else (
            goto :loopEnd
        )
    )
    :loopEnd
    if not exist !filePath! (
        call :LogErr "Create file !filePath! failed."
    )
goto :EOF

:CheckMountPath
    mountvol !ParamMountPath! /L
    exit /b !errorlevel!
goto :EOF

:MountDataturboFs
    set retryTime=3
    for /l %%a in (1 1 !retryTime!) do (
        dataturbo mount storage_object storage_name=!ParamStorageName! filesystem_name=!ParamNasFileSystemName! mount_dir=!ParamMountPath! >> !LOGFILEPATH! 2>&1
        if not "!errorlevel!" == "0" (
            call :LogErr "Mount !ParamStorageName! !ParamNasFileSystemName! !ParamMountPath! failed retry %%a time"
            call :WinSleep 5
        ) else (
            goto :loopEnd
        )
    )
    :loopEnd
    call :CheckMountPath
    if not "!errorlevel!" == "0" (
        call :LogErr "Mount failed, !ParamNasFileSystemName! is not exist.."
        set RET_CODE=!ERROR_SCRIPT_EXEC_FAILED!
        goto :end
    )
    call :LogInfo "!ParamNasFileSystemName! mount success."
goto :EOF

:LogInfo
    call :Log "[INFO] %~1"
goto :EOF

:LogErr
    call :Log "[ERR] %~1"
goto :EOF

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
				more %CURRENTCMDLineRST% | findstr /c:"dataturbo_mount.bat %AGENT_ROOT% %PID%" 1>nul 2>&1
			) else (
				more %CURRENTCMDLineRST% | findstr /c:"dataturbo_mount.bat\" \"%AGENT_ROOT%\" %PID%" 1>nul 2>&1
			)
			if !errorlevel! EQU 0 (
				set /a CURRENTPID=%%a
				goto :EOF
			)
		)
		set /a NUM=!NUM!+1
	)
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof

:end
    call :Log "Finish !OperationType! dataturbo filesystem."
    exit !RET_CODE!
goto :EOF

endlocal