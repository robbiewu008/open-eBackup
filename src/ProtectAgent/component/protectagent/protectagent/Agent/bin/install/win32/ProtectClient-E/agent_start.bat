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
::The script is used to start Agent Service.

set PARAM1=%~1

set CURRENT=%~dp0
set AGENT_ROOT=%CURRENT%..\
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%log\
set LOGFILE_PATH=%AGENT_LOG_PATH%agent_start.log
set WORKING_USER=rdadmin
rem WORKING_USER_PWD is used for changing user. It can be any value but not empty.
set WORKING_USER_PWD=pwd
set DS_REPAIR_MODE=DsRepair
set NGINX_SERVICE_NAME=rdnginx
set NEED_LOG=1
set IS_UPGRADE=0
set NUMBER=0

echo Begin start DataBackup ProtectAgent services...
echo.

if "%PARAM1%" == "dataturbo" (
    rem start dataturbo service
    call :startservice dataturbo
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

rem Check windows satrt status. We need to support directory repair mode.
call :ChecksystemStartStatus
if not "!errorlevel!" == "0" (exit /b 1)

if NOT "%PARAM1%" EQU "" (
    if "%PARAM1%" EQU "upgrade" (
        set IS_UPGRADE=1
    )
)

if "%PARAM1%" == "rdagent" (
    rem start rdagent service
    call :startservice rdagent
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

if "%PARAM1%" == "rdnginx" (
    rem start rdnginx service
    call :startservice rdnginx
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

if "%PARAM1%" == "rdmonitor" (
    rem start rdmonitor service
    call :startservice rdmonitor
    if not "!errorlevel!" == "0" (exit /b 1)
    exit /b 0
)

rem start rdagent service
call :startservice rdagent
if not "!errorlevel!" == "0" (exit /b 1)

rem start rdnginx service
call :startservice rdmonitor
if not "!errorlevel!" == "0" (exit /b 1)

rem start rdmonitor service
set RETRY_NUMBER=0
:retrystartnginx
call :startservice rdnginx
if not "!errorlevel!" == "0" (
    if !RETRY_NUMBER! LSS 3 (
        set PARAM_NAME=listen
        call :modifyconffile
        set /a RETRY_NUMBER+=1
        call :Log "Start service !SERVICE_NAME! failed, and retry !RETRY_NUMBER! times after 3 sec."
        goto :retrystartnginx
    )
    call :Log "Start service !SERVICE_NAME! failed."
    exit /b 1
)

echo.
echo DataBackup ProtectAgent services was started successfully.
call :Log "DataBackup ProtectAgent services was started successfully."

call :Winsleep 3

endlocal
exit /b 0

:startservice
    set SERVICE_CHECK=
    set SERVICE_NAME=%~1
    set RETRY_COUNT=1
    set SC_RETRY_COUNT=1
    
    call :checkservice !SERVICE_NAME!
    if %errorlevel% EQU 2 (
        call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent is not exist, it is a normal situation during the upgrade process."
        exit /b 0
    )
    if not "!errorlevel!" == "0" (
        echo The service !SERVICE_NAME! of DataBackup ProtectAgent is not exist.
        call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent is not exist."
		call :Winsleep 3
        exit /b 1
    )
    
    for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
    if not "!SERVICE_CHECK!" == "" (
    
        echo The service !SERVICE_NAME! of DataBackup ProtectAgent is already exist.
        call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent is already exist."
        
    ) else (
    
        :startagentservice
            sc start !SERVICE_NAME! >>"%LOGFILE_PATH%" 2>&1
            
            :waitstart
                call :Winsleep 3
                
                set SERVICE_CHECK=
                for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
                
                if not "!SERVICE_CHECK!" == "" (
                
                    echo The service !SERVICE_NAME! of DataBackup ProtectAgent is running now.
                    call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent is running now."
                    if "!SERVICE_NAME!" == "rdagent" (
                        set PARAM_NAME=fastcgi_pass
                        call :modifyconffile
                    )
                ) else (
                    rem retry 5 times 
                    if !RETRY_COUNT! LEQ 5 (
                        set /a RETRY_COUNT+=1
                        call :Log "Wait the service !SERVICE_NAME! status is stopped, and retry !RETRY_COUNT! times after 3 sec."
                        goto :waitstart
                    )

                    if !SC_RETRY_COUNT! LEQ 3 (
                        set /a SC_RETRY_COUNT+=1
                        call :Log "start service !SERVICE_NAME! faild, and retry !SC_RETRY_COUNT! times after 15 sec."
                        set RETRY_COUNT=1
                        goto :startagentservice
                    )
                    
                    echo The service !SERVICE_NAME! of DataBackup ProtectAgent was started failed.
                    call :Log "The service !SERVICE_NAME! of DataBackup ProtectAgent was started failed after retry 3 times, exit 1."
                    call :Winsleep 3
                    exit /b 1
                    
                )
    )

    exit /b 0
    
:checkservice
    call :Log "Check Service %~1."
    set CHECK=
    for /f "delims=" %%i in ('2^>nul sc query %~1 ^| findstr /i "%~1"') do (set CHECK=%%i)
    if "!CHECK!" == "" (
        if %IS_UPGRADE% EQU 1 (
            rem This is a normal situation during the upgrade process
            exit /b 2
        )
        rem Service %~1 is not exist.
        exit /b 1
    )
    rem Service %~1 is exist
    exit /b 0

:ChecksystemStartStatus
    set STATUS=
    for /f "tokens=2 delims=: " %%a in ('bcdedit') do (
        if "%%a"=="%DS_REPAIR_MODE%" (
            set STATUS=%DS_REPAIR_MODE%
        )
    )

    set USER_NAME=
    for /f "tokens=2 delims='='" %%a in ('wmic service where "name='%NGINX_SERVICE_NAME%'" get startname /value') do (
        set USER_NAME=%%a
    )
    rem delete ".\" before username
    set USER_NAME=%USER_NAME:.\=%

    rem windows2008: rdagent service user is rdadmin
    set WIN_RELEASE_VERSION=
    for /f "delims=" %%i in ('"!AGENT_BIN_PATH!xmlcfg.exe" read System win_version') do (set WIN_RELEASE_VERSION=%%i)

    if not "!STATUS!" == "%DS_REPAIR_MODE%" (
        if not "!USER_NAME!" == "%WORKING_USER%" (
            echo "Normal boot start!"
            call :Log "Normal boot start!"
            if "!WIN_RELEASE_VERSION!" == "6.1" (
                call "%AGENT_BIN_PATH%\winservice.exe" rdagent changeUser !WORKING_USER! !WORKING_USER_PWD!
                call :Log "Set working user of rdagent process rdadmin."
            ) 
            call "%AGENT_BIN_PATH%\winservice.exe" nginx changeUser !WORKING_USER! !WORKING_USER_PWD!
        )
    ) else (
        if "!USER_NAME!" == "%WORKING_USER%" (
            echo "Directory repair mode start!"
            call :Log "Directory repair mode start!"
            call "%AGENT_BIN_PATH%\winservice.exe" nginx changeUser
            if "!WIN_RELEASE_VERSION!" == "6.1" (
                call "%AGENT_BIN_PATH%\winservice.exe" rdagent changeUser
            )
        )
    )

    if not "!errorlevel!" == "0" (
        call :Log "Change user of service failed!"
        exit /b 1
    )
    exit /b 0

:GetRandomPort
    set BeginSeq=%~1
    set FinalSeq=%~2
    set PotrUser=%~3
    set PortTemp=0
 
    set /a Diff=!FinalSeq! - !BeginSeq!
    set /a PortTemp=!BeginSeq!
    if "!Diff!" == "0" (
        call :Log "Diff is 0."
        set /a PortTemp=!BeginSeq! + !NUMBER!
    ) else (
        set /a PortTemp=!random! %% !Diff! + !BeginSeq!
        if not "!errorlevel!" == "0" (
            call :Log "Get randomport failed."
            set /a PortTemp=!BeginSeq! + !NUMBER!
        )
    )

	if "!PotrUser!" == "rdagent" ( set AGENT_PORT=!PortTemp!)
    if not "!errorlevel!" == "0" ( 
        echo set agent port failed.
        call :Log "Set agent port failed."
		exit /b 1
    )
    if "!PotrUser!" == "nginx" ( set NGINX_PORT=!PortTemp!)
    if not "!errorlevel!" == "0" ( 
        echo set nginx port failed.
        call :Log "Set nginx port failed."
		exit /b 1
    )
	exit /b 0

:setports
        set PORT_NUM=
		
		if "%NUMBER%" LSS "5" (
		    set PORT_NUM=
            if "%~1" == "rdagent" (
		        call :GetRandomPort 59540 59559 rdagent
			    if not "!errorlevel!" == "0" ( set AGENT_PORT=8091 )
			    set PORT_NUM=!AGENT_PORT! 
		    )
		    if "%~1" == "nginx" (
		        call :GetRandomPort 59520 59539 nginx
			    if not "!errorlevel!" == "0" ( set NGINX_PORT=59526 )
			    set PORT_NUM=!NGINX_PORT!
		    )
            if not "!errorlevel!" == "0" (
		        echo Get random port failed.
				call :Log "Get random port failed."
			    exit /b 1
		    )
        
			set PORT_EXIST=
			set PORT_FLAG=0
			for /f "tokens=1,2,3* delims=: " %%a in ('2^>nul netstat -an ^| findstr !PORT_NUM!') do (
				if "%%c" == "!PORT_NUM!" (
					set PORT_FLAG=1
				)
			)
			 
			if !PORT_FLAG! EQU 1 (
				echo The port number is used by other process!
				set /a NUMBER+=1
				ping -n 2 127.0>nul
				goto :setports
			)
		) else (
            echo The !PORT_NAME! port reset failed 5 times, exit.
			call :Log "The !PORT_NAME! port reset failed 5 times, exit."
			exit /b 1
        )
		echo The !PORT_NAME! port reset.
		call :Log "The !PORT_NAME! reset."
		exit /b 0

:modifyconffile
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "!AGENT_ROOT!nginx\conf\nginx.conf"') do (
        for /f %%j in ('2^>nul echo.%%b ^| findstr  !PARAM_NAME!') do (
            set IFLAF_NUM=%%a 
        )
    )

    if !IFLAF_NUM! equ 0 (
        call :Log "Don't find the !PARAM_NAME! in the nginx.conf, exit 1."
        %CMD_PAUSE%
        exit 1
    )

    if "!PARAM_NAME!" == "fastcgi_pass" (
        for /f "delims=" %%i in ('"!AGENT_BIN_PATH!xmlcfg.exe" read System port') do (set PORT_NUM=%%i)
            if "!PORT_NUM!" == "" (
            call :Log "Query agent prot number failed in previous Agent."
            exit 1
        )
    )

    set host_ip=
    if "!PARAM_NAME!" == "listen" (
        for /f "tokens=3 delims= " %%a in ('findstr /n "listen" "!AGENT_ROOT!\nginx\conf\nginx.conf"') do (
            echo %%a | findstr "]" >nul && (
	            for /f "delims=]" %%i in ('2^>nul echo.%%a ') do (
	                set host_ip=%%i]
                )
	        ) || (
                for /f "delims=:" %%i in ('2^>nul echo.%%a ') do (
	                set host_ip=%%i
                )
            )
            call :Log "nginx listen ip is:!host_ip!."
        )

        call :Log "Start reset nginx port."
        set /a NUMBER=0
        set PORT_NUM=
        set PORT_NAME=nginx
        call :setports !PORT_NAME! 
    )

    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "!AGENT_ROOT!\nginx\conf\nginx.conf"') do (
        if %%a equ %IFLAF_NUM% (
            if "!PARAM_NAME!" == "listen" (
                echo.        !PARAM_NAME!       !host_ip!:!PORT_NUM! ssl;>>"!AGENT_ROOT!nginx\conf\nginx.conf.bak1"  
            ) else (
                echo.            !PARAM_NAME!   127.0.0.1:!PORT_NUM!;>>"!AGENT_ROOT!nginx\conf\nginx.conf.bak1"
            )
        ) else (
            echo.%%b >>"!AGENT_ROOT!nginx\conf\nginx.conf.bak1"
        )   
    )
    MOVE "!AGENT_ROOT!nginx\conf\nginx.conf.bak1" "!AGENT_ROOT!nginx\conf\nginx.conf" >nul
    call :Log "Set %PORT_NAME% port %PORT_NUM% successfully."
    goto :EOF

:Winsleep
    ping 127.0.0.1 -n %~1 > nul
    exit /b 0

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
    )
    
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILE_PATH%
    
    goto :EOF
