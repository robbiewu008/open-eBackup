@echo off
rem @dest:   application agent for cifs
rem @date:   2022-06-18
rem @authr:  kWX884906
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
set MountDriver=W
set WIN_SYSTEM_DISK=%WINDIR:~0,1%

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
call :ProcessIp !StorageIp!

call :Log "JobID=%JobID%;OperationType=%OperationType%;LinkPath=%LinkPath%;SubPath=%SubPath%;StorageIp=%StorageIp%;FilesystemName=%FilesystemName%;AuthKey=%AuthKey%;MountDriver=%MountDriver%;SharedPath=%SharedPath%"

call :Log "WIN_SYSTEM_DISK=!WIN_SYSTEM_DISK!."
if "!WIN_SYSTEM_DISK!" == "C" (
	call :Log "Enter normal mode."
	if "!OperationType!" == "mount" (
		call :Mount
	) else if "!OperationType!" == "unmount" (
		call :ProcessSharedPath
		call :Unmount
	) else (
		call :Log "error OperationType=!OperationType!."
		set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
	)
) else (
	call :Log "Enter PE mode."
	if "!OperationType!" == "mount" (
		call :MountPE
	) else if "!OperationType!" == "unmount" (
		call :UnmountPE
	) else (
		call :Log "error OperationType=!OperationType!.(PE)"
		set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
	)
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

:ProcessIp
	set "ip=%~1"
	set "is_ipv4=1"
	echo !ip! | findstr "\." > nul && (
		set IP_TYPE=IPV4
	) || (
		set IP_TYPE=IPV6
	)
	:break_loop
		if "!IP_TYPE!" EQU "IPV4" (
			set "StorageIp=!ip!"
		) else (
			set "temp_ip=!ip!"
			set "temp_ip=!temp_ip::=-!"
			set "StorageIp=!temp_ip!.ipv6-literal.net"
		)
	call :Log "ProcessIp = !StorageIp!"
goto :EOF

:ProcessSharedPath
	for /f "tokens=1* delims=\" %%a in ("%SharedPath%") do (
    	set "StorageIp=%%a"
    	set "remaining=%%b"
	)
	call :ProcessIp !StorageIp!
	set SharedPath=\\!StorageIp!\!remaining!
	call :Log "ProcessSharedPath = !SharedPath!"
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

:AfterMountSuccessPE
	if exist !MountDriver!\rdagent_jobid.txt (
		attrib "!MountDriver!\rdagent_jobid.txt" -s -h
	)
	echo !JobID! > !MountDriver!\rdagent_jobid.txt
	attrib "!MountDriver!\rdagent_jobid.txt" +s +h

	echo !MountDriver!> !RSTFILE!

	if not "!SubPath!" == "" (
		set SubPath=!SubPath::= !
		for %%a in (!SubPath!) do (
			if not exist !MountDriver!\%%a (
			   md !MountDriver!\%%a
			   call :Log "md !MountDriver!\%%a."
			)
		)
	)
goto :EOF

:FindMountDriver
    rem get unused disk driver
    set str=W V U T S R Q P O N M L K J I H G F E D C
    for %%i in (!str!) do (
        if not exist %%i: (
            set MountDriver=%%i:
			net use !MountDriver! >> %LOGFILEPATH% 2>nul
			if "!errorlevel!"=="0" (
				call :Log "Disk !MountDriver! is exist, but unavailable, umount !MountDriver!."
				net use !MountDriver! /del /y >> %LOGFILEPATH% 2>&1
				net use !MountDriver! >> %LOGFILEPATH% 2>nul
				if "!errorlevel!"=="0" (
					call :Log "Unmount failed, !MountDriver! is still exist, find next unused disk driver."
					set MountDriver=
				) else (
					goto :FindMountDriverEnd
				)
			) else (
				goto :FindMountDriverEnd
			)
        )
    )

	:FindMountDriverEnd
    if "!MountDriver!" == "" (
        call :Log "there is no unused disk driver."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :end
    ) else (
        call :Log "find unused log disk driver !MountDriver!."
    )
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

:CheckMountPE
	for /f "tokens=1-3 delims= " %%a in ('net use ^| findstr \\\\%StorageIp%\\%FilesystemName%') do (
        set MountDriver=%%b
    )

	if not "!MountDriver!" == "" (
        call :Log "\\!StorageIp!\!FilesystemName! is already mount on !MountDriver!.(PE)"
		call :AfterMountSuccessPE
        goto :end
    )
goto :EOF

:Mount
    set MountDriver=

	for /f "tokens=2 delims=:" %%a in ('chcp') do (
        set oldCodedFormat=%%a
    )
	chcp 65001
	call :CheckMount
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

:MountPE
	set MountDriver=
    call :CheckMountPE
    call :FindMountDriver

	for /f "tokens=2 delims=:" %%a in ('chcp') do (
        set oldCodedFormat=%%a
    )
	chcp 65001
	for /l %%i in (1,1,3) do (
        call !AGENT_BIN_PATH!\agentcli.exe MountCifs "net use !MountDriver! \\!StorageIp!\!FilesystemName! \"*\" /user:localdomain\!AuthKey! /PERSISTENT:YES" >> %LOGFILEPATH% 2>&1
        if "!errorlevel!"=="0" (
            goto :loopEnd
        )
        call :WinSleep 5
    )
    :loopEnd
    chcp !oldCodedFormat!

    net use | findstr \\\\%StorageIp%\\%FilesystemName% 1>nul
    if not "!errorlevel!"=="0" (
        call :Log "MountPE failed, \\!StorageIp!\!FilesystemName! is not exist."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :end
    )
	call :Log "\\!StorageIp!\!FilesystemName! success mount on !MountDriver!.(PE)"
	call :AfterMountSuccessPE
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

:UnmountPE
    net use !LinkPath:~0,2! /del /y >> %LOGFILEPATH% 2>&1
    net use !LinkPath:~0,2! >> %LOGFILEPATH% 2>&1
    if "!errorlevel!"=="0" (
        call :Log "UnmountPE failed, !LinkPath! is still exist."
        set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
        goto :end
    )
	call :Log "UnmountPE !LinkPath!: success."
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof

:end
    call :Log "Finish !OperationType! cifs filesystem."
	if "!ERR_CODE!" == "" (
		exit 0
	) else (
    	exit !ERR_CODE!
	)

endlocal
