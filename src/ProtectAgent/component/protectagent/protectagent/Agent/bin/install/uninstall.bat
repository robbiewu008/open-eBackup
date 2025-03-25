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
rem #######################################################
rem #
rem #  upgrade: %~1=/q  push upgrade: %~1=/r
rem #   
rem #  %~2: "1":not del working user; others del
rem #
rem #######################################################

rem -----------------------------------------------------------
rem the small pkg uninstall(AI_SHU)
rem -----------------------------------------------------------
setlocal EnableDelayedExpansion

echo ********************************************************
echo      Begin to uninstall DataBackup ProtectAgent     
echo ********************************************************
 
set DATA_BACKUP_AGENT_HOME_VAR=
for /f "tokens=2 delims==" %%a in ('wmic environment where "name='DATA_BACKUP_AGENT_HOME' and username='<system>'" get VariableValue /value') do (
    if not "%%a" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%%a
    )
)
if "%DATA_BACKUP_AGENT_HOME_VAR%" == "" (
    echo DATA_BACKUP_AGENT_HOME not defined
    set DATA_BACKUP_AGENT_HOME_VAR=C:
) else (
    echo "DataBackup Agent is installed in the directory %DATA_BACKUP_AGENT_HOME_VAR%."
)

set AGENT_INSTALL_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\ProtectClient\
set AGENT_ERR_LOG_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\var\log\ProtectAgent
set AGENT_PLUGIN_PATH=%AGENT_INSTALL_PATH%Plugins
set AGENT_ROOT_PATH=%AGENT_INSTALL_PATH%ProtectClient-E\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%log\
set PROTECT_BIN_PATH=%AGENT_INSTALL_PATH%ProtectClient-E\bin\
set LOGFILE_PATH=%AGENT_LOG_PATH%uninstall.log
set SESERV_LOGON_RIGHT_FILE=%AGENT_ROOT_PATH%tmp\logonrightinfo
set UNINSTALL_TMP_FILE=%AGENT_ROOT_PATH%tmp\uninstalltmpinfo
set SESERV_LOGON_RIGHT_TMP_DB=%AGENT_ROOT_PATH%tmp\logonright.sdb
set PROCESS_INFO_PATH=%AGENT_ROOT_PATH%tmp\processinfo
set REGEDIT_PROFILELIST_PATH=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\
set DATATURBO_FUNC="%PROTECT_BIN_PATH%dataturbo_func.bat"

for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set Current_Time=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%
set AGENT_OLD_LOG_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\var\log\ProtectAgent%Current_Time%

set CMD_PAUSE=pause
set AGENT_USER=rdadmin
set PRODUCT_AGENT_NAME=DataBackup Agent

rem ----------Checking Administrator Permissions--------------- 
set UPGRADE_VALUE=
if "%~1" == "upgrade" (
    set /a UPGRADE_VALUE=1
	set CMD_PAUSE=
)
net.exe session 1>NUL 2>NUL || (
    echo Please run the script as an administrator.
	call :Log "Please run the script as an administrator."
	call :gotoexit
    %CMD_PAUSE%
    exit /b 1
)

rem ---------------Begin uninstall Agent-------------------------
call :Log "########################Begin uninstall Agent########################"
echo You are about to uninstall the %PRODUCT_AGENT_NAME%. This operation stops the %PRODUCT_AGENT_NAME% service and deletes the %PRODUCT_AGENT_NAME% and customized configuration data which cannot be recovered. Therefore, applications on the host are no longer protected.
echo.
echo Suggestion: Confirm whether the customized configuration data, such as customized script, has been backed up.
echo.

if "!UPGRADE_VALUE!" == "1" (
    call :getinput upgrade
) else (
    call :getinput 
)

if not "!errorlevel!" == "0" (
    call :gotoexit
    exit /b 1
)

rem -----------------Uninstall the dataturbo--------------------
if NOT "!UPGRADE_VALUE!" == "1" (
    call %DATATURBO_FUNC% UninstallDataTurbo "%LOGFILE_PATH%" "%CURRENT_PATH%"
    if NOT "!errorlevel!" == "0" (
        call :Log "Uninstall dataturbo failed, need manually uninstall it."
    )
)

if "%~1" == "upgrade" (
    call :uninstallagent upgrade
) else (
    call :uninstallagent 
)

if not "!errorlevel!" == "0" (
    echo %PRODUCT_AGENT_NAME% was uninstalled failed.
    call :Log "%PRODUCT_AGENT_NAME% was uninstalled failed."
    goto :gotoexit
    exit /b 1
)

rem ------------------ Kill process ---------------------------
call :ForceKillProcess monitor.exe
call :ForceKillProcess rdagent.exe
call :ForceKillProcess rdnginx.exe

rem ------------------ Delete environment variable--------------------
if "!UPGRADE_VALUE!" == "1" (
    call :Log "Environment variable will not be deleted during the upgrade process."
) else (
    wmic ENVIRONMENT where "name='DATA_BACKUP_AGENT_HOME'" delete >> %LOGFILE_PATH%
    call :Log "Environment variable have been deleted."
)

call :collectloginuninstall

rem ------------------ Remove the installation directory--------------
echo Start remove install dir.
cd C:\
rmdir /s /q %AGENT_ROOT_PATH% %AGENT_PLUGIN_PATH%
if not "!errorlevel!" == "0" ( 
    echo Remove install dir failed,%PRODUCT_AGENT_NAME% has been uninstalled failed.
    call :Log "Remove install dir failed,%PRODUCT_AGENT_NAME% has been uninstalled failed."
    call :gotoexit
    exit /b 1
)
echo %PRODUCT_AGENT_NAME% has been uninstalled successfully, the applications on the host are no longer protected.
if "!UPGRADE_VALUE!" == "1" (
    exit /b 0
)
%CMD_PAUSE%
exit /b 0

rem ----------------------------------------step end --------------------------------------------------

:getinput
    set IFLAG=1
    echo Are you sure you want to uninstall %PRODUCT_AGENT_NAME%? ^(y/n, default:n^):
    if "%~1" == "upgrade" (
	echo The DataBackup ProtectAgent will be uninstalled.
	exit /b 0 )
	set /p UNINSTALL=">>"
	
    if "!UNINSTALL!" == "" (
		exit /b 1
    )
    
    if "!UNINSTALL!" == "y" (
        set IFLAG=0
    )

    if "!UNINSTALL!" == "n" (
        set IFLAG=0
    )

    if "!IFLAG!" == "1" (
        set /a NUMBER+=1
        if !NUMBER! LSS 3 (
            echo Please enter y or n.
            goto :getinput
        ) else (
            echo Input invalid value over 3 times.
			call :Log "Input invalid value over 3 times."
			exit /b 1
        )
    )
    
    if "!UNINSTALL!" == "n" (
		exit /b 1
    )
    
    exit /b 0

:uninstallagent
    echo Begin to uninstall %PRODUCT_AGENT_NAME%...
    echo.
    
    if not exist "%PROTECT_BIN_PATH%\winservice.exe" (
        echo File winservice.exe is not exist.
        call :Log "File winservice.exe is not exist, then exit 1."
        exit /b 1
    )
    
    call :unregistServices rdmonitor monitor
    if not "!errorlevel!" == "0" (exit /b 1)
    
    call :unregistServices rdnginx nginx
    if not "!errorlevel!" == "0" (exit /b 1)
    
    call :unregistServices rdagent rdagent
    if not "!errorlevel!" == "0" (exit /b 1)
    
    if "%~1" == "upgrade" (
        echo The upgrade process does not require users and groups to be reset.			
    ) else (
        call :deleteworkinguser !AGENT_USER!
        if not "!errorlevel!" == "0" (exit /b 1)
    )

    call :uninstallPlugins
	if not "!errorlevel!" == "0" (exit /b 1)

    rem The upgrade process does not report offline requests
	if "%UPGRADE_VALUE%" == "1" (exit /b 0)

    call :delHostFromPM
    if not "!errorlevel!" == "0" (exit /b 1)
	exit /b 0

rem -----------------
rem %~1:SERVICE_NAME
rem %~2:SERVICE_PARAM
rem -----------------  
:killprocess
    set processparm=%~1
    tasklist >> "!PROCESS_INFO_PATH!"
    echo start to execute kill !processparm!
    set /a retry = 1
    for /l %%i in (1,1,3) do (
        for /f "tokens=1,2,3* delims= " %%a in ( 'type "!PROCESS_INFO_PATH!" ^| findstr "!processparm!"') do ( set killparam=%%b )
        if not "!errorlevel!" == "0" ( 
            echo Get process pid failed.
            exit /b 0
        )
        if not "!killparam!" == "" (
            call :Log "uinstall-killprocess: process=!processparm!, pid=!killparam!, begin to kill it."
            taskkill /pid !killparam! /F >nul 
            if "!UPGRADE_VALUE!" == "1" (
                call :WinSleep 5
                del /f /q "%PROCESS_INFO_PATH%"
                exit /b 0
            )
            if "!errorlevel!" == "0" (
                echo kill !processparm! success
                goto :loopEnd
            )
            echo kill failed, retry !retry! time
            set /a retry = retry + 1
            call :WinSleep 5
        ) else (
            call :Log "process=!processparm! isn't running."
        )
    )
    :loopEnd
    if not "!errorlevel!" == "0" ( 
        echo Kill process failed.
        exit /b 1
    )
    call :WinSleep 5
    del /f /q "%PROCESS_INFO_PATH%"
    exit /b 0

:delHostFromPM
echo Start reporting offline requests.
call :Log "Start reporting offline requests."

call "%PROTECT_BIN_PATH%agentcli.exe" registerHost DeleteHost >> "%LOGFILE_PATH%" 2>&1
if not "!errorlevel!" == "0" (
    echo Delete host to ProtectManager failed.
	call :Log "Delete host to ProtectManager failed."
    call :gotoexit
	exit /b 1
)
exit /b 0

:uninstallPlugins
    for /f "delims=" %%a in ('dir !AGENT_PLUGIN_PATH! /b /a:d') do (
        set PLUGIN_NAME=%%a
        if exist "!AGENT_PLUGIN_PATH!\!PLUGIN_NAME!\uninstall.bat" (
            echo Uninstall plugin !PLUGIN_NAME!.
            call :Log "Uninstall plugin !PLUGIN_NAME!."
            call "!AGENT_PLUGIN_PATH!\!PLUGIN_NAME!\uninstall.bat"
            if not "!errorlevel!" == "0" (
                echo Uninstall Plugins failed.
                call :Log "Uninstall Plugins failed."
                exit /b 1
            )
        )
    )
    exit /b 0

:unregistServices
    set SERVER_QUERY=
    set SERVICE_NAME=%~1
    set SERVICE_PARAM=%~2
    for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set SERVER_QUERY=%%b)
    if "!SERVER_QUERY!" == "" (
        echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% is not exist, no need uninstall.
        call :Log "Process !SERVICE_NAME! of %PRODUCT_AGENT_NAME% is not exist, no need uninstall."
        exit /b 0
    )

	set TMP_NUM=0
    
	set SERVICE_CHECK=
    for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
	if not "!SERVICE_CHECK!" == "" (
	    if "!SERVICE_NAME!" == "rdmonitor" (
	        call :killprocess monitor.exe
		    if not "!errorlevel!" == "0" ( exit /b !errorlevel! )
	    )
        call :WinSleep 10
        if "!SERVICE_NAME!" == "rdnginx" (
            call :killprocess rdnginx.exe
            if not "!errorlevel!" == "0" ( exit /b !errorlevel! )
        )
        if "!SERVICE_NAME!" == "rdagent" (
	        call :killprocess rdagent.exe
		    if not "!errorlevel!" == "0" ( exit /b !errorlevel! )
	    )
        sc stop %SERVICE_NAME% >> "%LOGFILE_PATH%"
	    call :WinSleep 3

	    if exist !PROCESS_INFO_PATH! del /q /f !PROCESS_INFO_PATH! >nul
	)
	if "!UPGRADE_VALUE!" == "1" (
	    echo Upgrade process doesn't need to uninstall service.
		call :Log "Upgrade process doesn't need to uninstall service."
		exit /b 0
	)
    call "%PROTECT_BIN_PATH%winservice.exe" !SERVICE_PARAM! uninstall
	for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set SERVER_QUERY=%%b)
    if not "!SERVER_QUERY!" == "" (
	    call "%PROTECT_BIN_PATH%winservice.exe" !SERVICE_PARAM! uninstall
	)
	
    if not "!errorlevel!" == "0" (
        call :WinSleep 3
        set OPER_RESULT=
        for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set OPER_RESULT=%%b)
        if "!OPER_RESULT!" == "" (
            echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully.
            call :Log "Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% successfully."
            exit /b 0
        )
        echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled failed.
        call :Log "Failed to execute service !SERVICE_NAME! uninstall command. Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% failed."
        exit /b 1
    ) else (
        call :WinSleep 3
        
        set OPER_RESULT=
        for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set OPER_RESULT=%%b)
        if "!OPER_RESULT!" == "" (
            echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully.
            call :Log "Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% successfully."
            exit /b 0
        ) else (
            echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled failed.
            call :Log "Check service !SERVICE_NAME! exist. Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% failed."
            exit /b 1
        )
    )
rem ---------------
rem %~1: Agent_user
rem --------------- 
:deleteworkinguser
    set WORKING_USER=%~1
    set InDomain=
    echo "inDomain is %inDomain%"
    if /i "!InDomain!"=="TRUE" (
        echo "Agent working user !WORKING_USER! exist in the domain, no need to uninstall."
        call :Log "Agent working user !WORKING_USER! exist in the domain, no need to uninstall."
        exit /b 0
    )
    echo.
    echo Delete user !WORKING_USER! of %PRODUCT_AGENT_NAME%...
    
    net user !WORKING_USER! 1>nul 2>nul
    if not "!errorlevel!" == "0" (
        call :Log "Agent working user !WORKING_USER! not exist, no need to delete."
        echo User !WORKING_USER! of %PRODUCT_AGENT_NAME% was Deleted successfully.
        exit /b 0
    )
    
    call :Log "Remove user !WORKING_USER! from Log on as a service."
    call :seservicelogonright
    if not "!errorlevel!" == "0" (exit /b 1) 

    for /f "tokens=2 delims==" %%a in ('wmic useraccount where "name='%WORKING_USER%'" get sid /format:list') do (set WORKING_USER_SID=%%a)
    call :deleteuserinfos
    if not "!errorlevel!" == "0" (
        call :Log "Cleanup userinfo sid:!WORKING_USER_SID! failed."
        echo Cleanup userinfo sid:!WORKING_USER_SID! failed.
        exit /b 1
    )

    net user !WORKING_USER! /delete >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Delete user !WORKING_USER! failed."
        echo User !WORKING_USER! of %PRODUCT_AGENT_NAME% was Deleted failed.
        exit /b 1
    )

    echo User !WORKING_USER! of %PRODUCT_AGENT_NAME% was Deleted successfully.
    call :Log "Delete user !WORKING_USER! succ."
    exit /b 0

:deleteuserinfos
    set WORKING_USER_PATH=
    if "%WORKING_USER_SID%" EQU "" (
        call :Log "Query user SID failed."
        exit /b 1
    )
    
    call :Log "Cleanup the user("%WORKING_USER%") sid(%WORKING_USER_SID%) homeDir and regeditor info."
    rem delete user home director
    for /f "tokens=1,2,*" %%a in ('2^>nul reg query "%REGEDIT_PROFILELIST_PATH%%WORKING_USER_SID%" ^| findstr /R "ProfileImagePath.*REG_EXPAND_SZ"') do (set WORKING_USER_PATH=%%c)
    if exist "%WORKING_USER_PATH%" (
        rd /s /q "%WORKING_USER_PATH%" 2>nul 1>>"%LOGFILE_PATH%"
    )
    if exist "%WORKING_USER_PATH%" (
        call :Log "Maybe the directory %WORKING_USER_PATH% may be occupied by another process."
    )
    call :Log "Delete the user("%WORKING_USER%") path(%WORKING_USER_PATH%) succ."

    rem clear regeditor
    for /f "tokens=*" %%a in ('2^>nul reg query "%REGEDIT_PROFILELIST_PATH%%WORKING_USER_SID%"') do (set USER_IS_IN_REGEDITOR=%%a)
    if not "%USER_IS_IN_REGEDITOR%" == "" (
        call :Log "Clear working user infos in regeditor (%REGEDIT_PROFILELIST_PATH%%WORKING_USER_SID%)."
        reg delete "%REGEDIT_PROFILELIST_PATH%%WORKING_USER_SID%" /f 2>& 1>>"%LOGFILE_PATH%"
        if "!errorlevel!" == "0" (
            call :Log "Clear working user infos in regeditor succ."
        ) else (
            call :Log "Clear working user infos in regeditor failed."
        )
    )
    
    exit /b 0

:seservicelogonright
    set UNICODE_VAL=
    set SIGNATURE_VAL=
    set REVISION_VAL=
    set SESLOR_VAL=
    set SEDRILOR_VAL=
    set SEDILOR_VAL=
    
    rem export seservicelogonright config
    if exist "!SESERV_LOGON_RIGHT_FILE!"  del /f /q "!SESERV_LOGON_RIGHT_FILE!"

    secedit /export /cfg "!UNINSTALL_TMP_FILE!" >> "%LOGFILE_PATH%" 2>&1
    if not "!errorlevel!" == "0" (
        call :Log "Export seservicelogonright config failed."
        if exist "!UNINSTALL_TMP_FILE!"  del /f /q "!UNINSTALL_TMP_FILE!"
        exit /b 1
    )
    
    rem get the info that Log on as a service need
    for /f "delims=" %%a in ('type "!UNINSTALL_TMP_FILE!"') do (
        set V_KEY=
        for /f "tokens=1 delims==" %%i in ('echo.%%a') do (set V_KEY=%%i)
        set V_KEY=!V_KEY: =!
        
        if "Unicode" == "!V_KEY!" (set UNICODE_VAL=%%a)

        if "signature" == "!V_KEY!" (set SIGNATURE_VAL=%%a)

        if "Revision" == "!V_KEY!" (set REVISION_VAL=%%a)
        
        if "SeServiceLogonRight" == "!V_KEY!" (
            set SESLOR_VAL=%%a
            set SESLOR_VAL=!SESLOR_VAL:,%WORKING_USER%=!
            set SESLOR_VAL=!SESLOR_VAL:%WORKING_USER%,=!
            set SESLOR_VAL=!SESLOR_VAL:%WORKING_USER%=!
        )
        
        if "SeDenyRemoteInteractiveLogonRight" == "!V_KEY!" (
            set SEDRILOR_VAL=%%a
            set SEDRILOR_VAL=!SEDRILOR_VAL:,%WORKING_USER%=!
            set SEDRILOR_VAL=!SEDRILOR_VAL:%WORKING_USER%,=!
            set SEDRILOR_VAL=!SEDRILOR_VAL:%WORKING_USER%=!
        )
        
        if "SeDenyInteractiveLogonRight" == "!V_KEY!" (
            set SEDILOR_VAL=%%a
            set SEDILOR_VAL=!SEDILOR_VAL:,%WORKING_USER%=!
            set SEDILOR_VAL=!SEDILOR_VAL:%WORKING_USER%,=!
            set SEDILOR_VAL=!SEDILOR_VAL:%WORKING_USER%=!
        )
    )
    
    echo.[Unicode]> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!UNICODE_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.[Version]>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SIGNATURE_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!REVISION_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.[Privilege Rights]>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SESLOR_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SEDRILOR_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SEDILOR_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    
    call :Log "get the infos that log on as a service need from the !UNINSTALL_TMP_FILE!"
    if exist "!UNINSTALL_TMP_FILE!"  del /f /q "!UNINSTALL_TMP_FILE!"
    
    rem import seservicelogonright config to .sdb file
    secedit /import /db "!SESERV_LOGON_RIGHT_TMP_DB!" /cfg "!SESERV_LOGON_RIGHT_FILE!" >> "%LOGFILE_PATH%" 2>&1 2>&1
    if not "!errorlevel!" == "0" (
        call :Log "Import the !SESERV_LOGON_RIGHT_FILE! to the !SESERV_LOGON_RIGHT_TMP_DB! failed."
        if exist "!SESERV_LOGON_RIGHT_FILE!"  del /f /q "!SESERV_LOGON_RIGHT_FILE!"
        exit /b 1
    )
    
    if exist "!SESERV_LOGON_RIGHT_FILE!"    del /f /q "!SESERV_LOGON_RIGHT_FILE!"
    
    rem add user seservicelogonright Log on as a service
    secedit /configure /db "!SESERV_LOGON_RIGHT_TMP_DB!" >> "%LOGFILE_PATH%" 2>&1
    if not "!errorlevel!" == "0" (
        call :Log "Add user !WORKING_USER! seservicelogonright to log on as a service failed."
        if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
        exit /b 1
    )
    
    if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
    call :Log "Remove user !WORKING_USER! seservicelogonright to log on as a service succ."
    
    exit /b 0

:ForceKillProcess
    for /f "tokens=1-6" %%a in (' tasklist ^| findstr "%~1" ') do (
        if not "%%a" EQU "" (
            if not "%%b" EQU "" (
                taskkill /f /t /pid %%b >> "%LOGFILE_PATH%" 2>&1
                call :WinSleep 1
                goto :ForceKillProcess
            ) else (
                call :Log "process %~1 isn't running."
            )
        )
        exit /b 0
    )
    exit /b 0

:Log
    echo %date:~0,10% %time:~0,8% [%username%] %~1 >> "%LOGFILE_PATH%"
    exit /b 0

:collectloginuninstall
    if exist "%AGENT_ERR_LOG_PATH%" (
        for /f %%i in ('dir /b "%AGENT_ERR_LOG_PATH%"') do set /a count+=1
        if !count! gtr 0 (
            xcopy /e/h/k/x/o/q/y "%AGENT_ERR_LOG_PATH%\*" "%AGENT_OLD_LOG_PATH%\" >> "%LOGFILE_PATH%" 2>&1
            del /f /s /q "%AGENT_ERR_LOG_PATH%\*.*" >> "%LOGFILE_PATH%" 2>&1
        )
	    if exist "%LOGFILE_PATH%" (
		    copy /y "%LOGFILE_PATH%" "%AGENT_ERR_LOG_PATH%" >> "%LOGFILE_PATH%" 2>&1
            copy /y "%AGENT_LOG_PATH%winservice.log" "%AGENT_ERR_LOG_PATH%" >> "%LOGFILE_PATH%" 2>&1
		)
	) else (
	    md "%AGENT_ERR_LOG_PATH%" >> "%LOGFILE_PATH%" 2>&1
		copy /y "%LOGFILE_PATH%" "%AGENT_ERR_LOG_PATH%" >> "%LOGFILE_PATH%" 2>&1
        copy /y "%AGENT_LOG_PATH%winservice.log" "%AGENT_ERR_LOG_PATH%" >> "%LOGFILE_PATH%" 2>&1
	)
    exit /b 0

:gotoexit
    if "!UNINSTALL!" NEQ "y" (
        echo "The uninstallation of %PRODUCT_AGENT_NAME% have been canceled."
        %CMD_PAUSE%
        exit /b 1
    )

    call :collectloginuninstall
	
	if exist "%AGENT_ROOT_PATH%" (
	    rmdir /s /q "%AGENT_ROOT_PATH%" >nul 2>&1
	)
	if exist "%AGENT_PLUGIN_PATH%" (
	    rmdir /s /q %AGENT_PLUGIN_PATH% >nul 2>&1
	)
    echo The uninstallation of %PRODUCT_AGENT_NAME% will be stopped.
    %CMD_PAUSE%
    exit /b 1
	
:WinSleep
    timeout /T %~1 /NOBREAK > nul
    exit /b 0

endlocal