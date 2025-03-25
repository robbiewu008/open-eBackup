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
rem ########################################################################
rem #
rem #  upgrade: %~1=/up; push upgrade: %~1=/up/r; 
rem #  %~2: "100":V100; "200":V200
rem #  %~3: /up : upgrade; /rb: rollback
rem #
rem #  push installation: %~1=push; upgrade installation: %~1=upgrade
rem #
rem ########################################################################

setlocal EnableDelayedExpansion

cd /d "%~dp0"

set LOGFILE_PATH=%AGENT_LOG_PATH%\agent_install.log

set INSTALL_TMP_FILE=%AGENT_ROOT_PATH%\tmp\installtmpinfo

set SESERV_LOGON_RIGHT_FILE=%AGENT_ROOT_PATH%\tmp\logonright.inf
set SESERV_LOGON_RIGHT_TMP_DB=%AGENT_ROOT_PATH%\tmp\logonright.sdb

set WIN08_VER=6

rem -------------error code-------------------
set ERR_WORKINGUSER_ADD_FAILED=1577209885
set ERR_WORKINGUSER_ADD_FAILED_RETCODE=28
set ERR_TMPINFO_FILE_EXIST=1577209882
set ERR_TMPINFO_FILE_EXIST_RETCODE=29
set ERR_SET_PMIP=1577210136
set ERR_SET_HOSTIP=1577210136
set ERR_SET_AGENT_PORT=1577210137
set ERR_MODIFY_AGENT_PORT=1577210137
set ERR_SET_NGINX_PORT=1577210137
set ERR_MODIFY_NGINX_PORT=1577210137
set ERR_WRITE_HOSTINFO_FAILED=36
set ERR_SERVICE_REGISTER_FAILED=1577210140
set ERR_REGISTER_TO_PM=1577210140
set ERR_REGISTER_TO_PM_RET_CODE=37
set ERR_GET_USER_PAW=1577209885
set ERR_GET_USER_PAW_RETCODE=39
set ERR_STOP_AGENT_SERVICE=1577210138
set ERR_STOP_AGENT_SERVICE_RETCODE=40
set ERR_START_AGENT_SERVICE=1577210140
set ERR_START_AGENT_SERVICE_RETCODE=41
set ERR_SET_PMIP_RETCODE=76
set ERR_SET_AGENT_PORT_RETCODE=77
set ERR_SERVICE_REGISTER_FAILED_RETCODE=80

set USER_REX="^[a-zA-Z][a-zA-Z0-9_]*$"
set IP_REX="^[0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*$"
set NUM_REX="^[1-9][0-9]*[\\s]*$"
set PATH_REX="^[-_\\sA-Za-z0-9:.\\]*$" 
set UPGRADE_VALUE=
if "%~1" == "upgrade" (
    set /a UPGRADE_VALUE=1
)

call :Log "AGENT_ROOT_PATH=%AGENT_ROOT_PATH%."

rem =================add working user=====================
echo Start add working user
set /a RETRY_TIMES=3
set WORKING_USER_PWD=
call :Log "Start add working user, type=%~1."
if "%UPGRADE_VALUE%" == "1" (
    echo The upgrade process does not require users and groups to be reset.
) else if "%~1" == "push" (
    set /p WORKING_USER_PWD=
    call :addworkinguser push
    if not "!errorlevel!" == "0" (
        call :LogError "add working user failed" %ERR_WORKINGUSER_ADD_FAILED%
        exit /b %ERR_WORKINGUSER_ADD_FAILED_RETCODE%
    )
) else (
    if /i "!InDomain!"=="TRUE" (
        echo "Use the !WORKING_USER! existed"
        call :Log "Use the !WORKING_USER! existed"
        call :addworkinguser manualdomain
    ) else (
        echo "Create the !WORKING_USER!"
        call :Log "Create the !WORKING_USER!"
        call :addworkinguser manual
    )
    if not "!errorlevel!" == "0" (
        call :LogError "add working user failed" %ERR_WORKINGUSER_ADD_FAILED%
        exit /b %ERR_WORKINGUSER_ADD_FAILED_RETCODE%
    )
)
call "%AGENT_BIN_PATH%\xmlcfg.exe" write System name !WORKING_USER!
call :Log "Add working user successfully."

rem =================get tmp info=========================
call :Log "Start parsing the parameter file testcfg.tmp."
set tempfile=
if exist "!AGENT_ROOT_PATH!\conf\testcfg.tmp" (
	set tempfile=!AGENT_ROOT_PATH!\conf\testcfg.tmp
	for /f "usebackq tokens=1,2 delims== " %%a in ( "!tempfile!" ) do (
		if "%%a" == "PKG_VERSION" (
			set PKG_VERSION=%%b
		)
		if "%%a" == "VERSION_TIME_STAMP" (
			set VERSION_TIME_STAMP=%%b
		)
		if "%%a" == "DOMAIN_NAME" (
			set DOMAIN_NAME=%%b
		)
		if "%%a" == "PM_IP" (
			set PMIP_LIST=%%b
		)
		if "%%a" == "PM_PORT" (
			set PM_PORT=%%b
		)
		if "%%a" == "BACKUP_ROLE" (
			set BACKUP_ROLE=%%b
		)
		if "%%a" == "USER_ID" (
			set USER_ID=%%b
		)
        if "%%a"=="eip" (
            set EIP=%%b
        )
	)

    call :checkIpsValid "!PMIP_LIST!"
    if NOT !errorlevel! EQU 0 (
        echo Business ip is not valid, use manage ip.
        call :Log "Business ip is not valid, use manage ip."
        for /f "usebackq tokens=1,2 delims== " %%a in ("!tempfile!") do (
            if "%%a"=="PM_MANAGER_IP" (
                set PMIP_LIST=%%b
            )
            if "%%a"=="PM_MANAGER_PORT" (
                set PM_PORT=%%b
            )
        )
    )
) else (
	echo The configuration file testcfg.tmp cannot be found.
    call :Log "The configuration file testcfg.tmp cannot be found."
    call :gotoexit
    call :LogError "The configuration file testcfg.tmp cannot be found." %ERR_TMPINFO_FILE_EXIST%
	exit /b %ERR_TMPINFO_FILE_EXIST_RETCODE%
)
echo Parsing the parameter file succeeded
call :Log "Parsing the parameter file succeeded."

rem ================set pm ip begin=======================
call :Log "Start set pm ip."
call :setpmip
if not "!errorlevel!" == "0" (
    call :LogError "set pm ip failed" %ERR_SET_PMIP%
    exit /b %ERR_SET_PMIP_RETCODE%
)

rem ================set host ip begin=====================
call :Log "Start set host ip."
set SYSTEM_VER=0
set /a NUMBER=0
for /f "tokens=1,2,3,4,5 delims=. " %%a in ('ver') do (set SYSTEM_VER=%%d)

if !SYSTEM_VER! LSS %WIN08_VER% (
    set IP_FILTER=ip.*:
) else (
    set IP_FILTER=ipv4
)

call :setipaddress
if not "!errorlevel!" == "0" (
    call :LogError "Set agent ip failed" %ERR_SET_HOSTIP%
    exit /b %ERR_SET_PMIP_RETCODE%
    )
echo Set host ip succeeded.

rem ================set agent port begin===============
call :Log "Start set agent port."
set /a NUMBER=0
set PORT_NUM=
set PORT_NAME=rdagent
call :setports !PORT_NAME! 
if not "!errorlevel!" == "0" (
    call :LogError "Set agent port failed" %ERR_SET_AGENT_PORT%
    exit /b %ERR_SET_AGENT_PORT_RETCODE%
    )

set PARAM_NAME=fastcgi_pass
call "%AGENT_BIN_PATH%\xmlcfg.exe" write System port !PORT_NUM!
if not "!errorlevel!" == "0" (
    call :Log "Set agent port number(!PORT_NUM!) failed."
    call :gotoexit
	exit /b 1
)
call :modifyconffile
if not "!errorlevel!" == "0" (
    call :LogError "Modify agent port failed" %ERR_MODIFY_AGENT_PORT%
    exit /b %ERR_SET_AGENT_PORT_RETCODE%
    )
call :Log "Set user agent port (!PORT_NUM!) successfully!"

rem ================set nginx port begin================
call :Log "Start set nginx port."
set /a NUMBER=0
set PORT_NUM=
set PORT_NAME=nginx
call :setports !PORT_NAME! 
if not "!errorlevel!" == "0" (
    call :LogError "Set nginx port failed" %ERR_SET_NGINX_PORT%
    exit /b %ERR_SET_AGENT_PORT_RETCODE%
    )

set PARAM_NAME=listen
call :checkhostip
call :modifyconffile
if not "!errorlevel!" == "0" (
    call :LogError "Modify nginx port failed" %ERR_MODIFY_NGINX_PORT%
    exit /b %ERR_SET_AGENT_PORT_RETCODE%
)

call :Log "Set user nginx port (!PORT_NUM!) successfully!"
rem ================set nginx port end==================

rem ===============start write hostinfo=================
call "%AGENT_BIN_PATH%\xmlcfg.exe" write System client_version !PKG_VERSION!
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration client_version."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration client_version modified successfully."

call "%AGENT_BIN_PATH%\xmlcfg.exe" write System version_time_stamp !VERSION_TIME_STAMP!
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration version_time_stamp."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration version_time_stamp modified successfully."

call "%AGENT_BIN_PATH%\xmlcfg.exe" write System domain_name !DOMAIN_NAME!
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration domain_name."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration domain_name modified successfully."

call "%AGENT_BIN_PATH%\xmlcfg.exe" write Backup ebk_server_ip !PM_IP!
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration ebk_server_ip."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration ebk_server_ip modified successfully."

call "%AGENT_BIN_PATH%\xmlcfg.exe" write Backup ebk_server_auth_port !PM_PORT! >> "%LOGFILE_PATH%"
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration ebk_server_auth_port!PM_PORT!."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration ebk_server_auth_port modified successfully."

call "%AGENT_BIN_PATH%\xmlcfg.exe" write Backup backup_role !BACKUP_ROLE!
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration backup_role [!BACKUP_ROLE!]."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration backup_role modified successfully [!BACKUP_ROLE!]."

call "%AGENT_BIN_PATH%\xmlcfg.exe" write System userid !USER_ID!
if not "!errorlevel!" == "0" (
    call :Log "Failed to modify the configuration userid."
    call :gotoexit
    exit /b %ERR_WRITE_HOSTINFO_FAILED%
)
call :Log "Configuration userid modified successfully."
rem ===================write hostinfo end===============

rem ================set file privilege begin============
echo Y | Cacls "!AGENT_MANAGER_PATH!" /T /E /R Users >nul 2>&1
rem ==============set file privilege end================

rem ==============start register to windows=============
call :Log "Start register service."
echo Start register service.
if not "%~1" == "upgrade" (
    rem These systems are included in version 6.1:
    rem Windows 7 RTM, Windows Server 2008 R2 RTM, Windows Server 2008 R2 SP1, Windows Home Server 2011
    if "!WIN_RELEASE_VERSION!" == "6.1" (
        call :registServices rdagent rdagent !WORKING_USER!
        call :Log "Set working user of rdagent process rdadmin."
    ) else (
        call :registServices rdagent rdagent
    )

    if not "!errorlevel!" == "0" ( 
        call :LogError "regist services failed" %ERR_SERVICE_REGISTER_FAILED%
        exit /b %ERR_SERVICE_REGISTER_FAILED_RETCODE% 
        )

    call :registServices rdnginx nginx !WORKING_USER!
    if not "!errorlevel!" == "0" ( 
        call :LogError "regist services failed" %ERR_SERVICE_REGISTER_FAILED%
        exit /b %ERR_SERVICE_REGISTER_FAILED_RETCODE% 
        )

    call :registServices rdmonitor monitor
    if not "!errorlevel!" == "0" (
        call :LogError "regist services failed" %ERR_SERVICE_REGISTER_FAILED%
        exit /b %ERR_SERVICE_REGISTER_FAILED_RETCODE% 
         )
) else (
    echo Upgrade process, modify the bin path of service.
    call :Log "Upgrade process, modify the bin path of service"

    call :modifyServiceBinPath rdagent "!AGENT_ROOT_PATH!\bin\rdagent.exe -s"
    if not "!errorlevel!" == "0" ( 
        call :LogError "regist services failed" %ERR_SERVICE_REGISTER_FAILED%
        exit /b %ERR_SERVICE_REGISTER_FAILED_RETCODE%
         )

    call :modifyServiceBinPath rdnginx "!AGENT_ROOT_PATH!\bin\winservice.exe nginx run"
    if not "!errorlevel!" == "0" (
        call :LogError "regist services failed" %ERR_SERVICE_REGISTER_FAILED%
        exit /b %ERR_SERVICE_REGISTER_FAILED_RETCODE%
        )

    call :modifyServiceBinPath rdmonitor "!AGENT_ROOT_PATH!\bin\monitor.exe -s"
    if not "!errorlevel!" == "0" ( 
        call :LogError "regist services failed" %ERR_SERVICE_REGISTER_FAILED%
        exit /b %ERR_SERVICE_REGISTER_FAILED_RETCODE%
         )
    
    sc config rdmonitor obj=LocalSystem
)
call :Log "Service register succeeded."
echo Service register succeeded.
rem ==============register to windows end=================

rem ==================copy HostSN========================
if exist "%AGENT_HOSTSN_PATH%" (
	copy "%AGENT_HOSTSN_PATH%\HostSN" "%AGENT_ROOT_PATH%\conf" >nul
) else (
	echo copy file HostSN failed.
    call :Log "copy file HostSN failed."
	call :unregisterall
	goto :gotoexit
	exit /b 1
)

rem ====================start Agent=======================
call :Log "Start to start the agent."
call "%AGENT_BIN_PATH%\agent_start.bat"
if not "!errorlevel!" == "0" (
    echo Start agent failed,start to clear source.
    call :Log "Start agent failed, start to clear source."
    if "%UPGRADE_VALUE%" == "1" (
        echo Upgrade process doesn't need uninstall service.	
    )
    call :unregisterall
    goto :gotoexit
    call :LogError "Start agent failed." %ERR_START_AGENT_SERVICE%
    exit /b %ERR_START_AGENT_SERVICE_RETCODE%
)
rem ====================start Agent end====================

rem ==================start register to pm=================
call :Log "Start regiser host to pm."
echo Start regiser host to pm.
if "%UPGRADE_VALUE%" == "1" (
    call "%AGENT_BIN_PATH%\agentcli.exe" registerHost RegisterHost Upgrade >> %LOGFILE_PATH% 2>&1
) else (
	call "%AGENT_BIN_PATH%\agentcli.exe" registerHost RegisterHost >> %LOGFILE_PATH% 2>&1
)
if not "!errorlevel!" == "0" (
    rem upgrade register failed, don't failed.
    if "%UPGRADE_VALUE%" == "1" (
        call :Log "This is upgrading process, Registering with the pm failed have no influnce, continue."
        echo This is upgrading process, Registering with the pm failed have no influnce, continue.
        exit /b 0
    ) else (
        echo "regiser host failed."
        call :Log "regiser host failed"
        call "%AGENT_BIN_PATH%\agent_stop.bat"
        if not "!errorlevel!" == "0" (
            echo Install Agent faile and stop service failed.
            call :LogError "Stop agent failed." %ERR_STOP_AGENT_SERVICE%
            exit /b %ERR_STOP_AGENT_SERVICE_RETCODE%
        )
        call :unregisterall
        goto :gotoexit
        call :LogError "regiser host to pm failed." %ERR_REGISTER_TO_PM%
        exit /b %ERR_REGISTER_TO_PM_RET_CODE%
    )
) 
call :Log "Registering with the pm succeeded."
echo Registering with the pm succeeded.
echo.
echo DataBackup Agent was installed successfully.
call :Log "DataBackup Agent was installed successfully."

exit /b 0
rem ==================register to pm end===================
:addworkinguser
    rem add user
    set OPRATE=%~1
    if !RETRY_TIMES! GTR 0 (
        echo Please enter password to set user !WORKING_USER!, you still have !RETRY_TIMES! chances:
        call :Log "Please enter password to set user !WORKING_USER!, you still have !RETRY_TIMES! chances:"
        call "!AGENT_BIN_PATH!\agentcli.exe" setuser !WORKING_USER! !OPRATE! !WORKING_USER_PWD!
        if not "!errorlevel!" == "0" (
            if "!OPRATE!" == "manualdomain" (
                echo The password is incorrect.
	            call :Log "The password is incorrect."
            ) else (
                echo The password is invalid.
	            call :Log "The password is invalid."
            )
            set /a RETRY_TIMES-=1
            goto :addworkinguser
        ) else (
            echo Set user !WORKING_USER! successfully.
            call :Log "Set user !WORKING_USER! succ."
        )
    ) else (
        if "!OPRATE!" == "manualdomain" (
            echo The password verification for the !WORKING_USER! account has failed more than three times. Please confirm the password. If this account has not been used before, please delete it manually.
            call :Log "The password verification for the !WORKING_USER! account has failed more than three times. Please confirm the password. If this account has not been used before, please delete it manually."
        ) else (
            echo Set user !WORKING_USER! failed.
            call :Log "Set user !WORKING_USER! failed."
        )
        goto :gotoexit
        exit /b 1
    )

    rem set password never expires
    wmic useraccount where "Name='!WORKING_USER!'" set PasswordExpires=FALSE >nul 2>&1
    call :Log "set user !WORKING_USER! succ."

    for /f "skip=1 delims= " %%a in ('wmic OS Get Locale') do (
        if "%%a" equ "040c" (
            net localgroup administrateurs !WORKING_USER! /add >nul 2>&1
            if not %errorlevel% == 0 (
                echo Add user !WORKING_USER! to localgroup administrators failed.
                call :Log "Add user !WORKING_USER! to localgroup administrators failed."
                goto :gotoexit
		        exit /b 1
                
            )
            goto :addFinish
        ) else (
            net localgroup administrators !WORKING_USER! /add >nul 2>&1
            if not %errorlevel% == 0 (
                echo Add user !WORKING_USER! to localgroup administrators failed.
                call :Log "Add user !WORKING_USER! to localgroup administrators failed."
                goto :gotoexit
		        exit /b 1
            )
            goto :addFinish
        )
    )
    :addFinish
	call :seservicelogonright
    if not "!errorlevel!" == "0" (
        goto :gotoexit
		exit /b 1
    )

    call :Log "Add user !WORKING_USER! to localgroup administrators succ."
    exit /b 0
	
:deleteworkinguser    
    net user !WORKING_USER! /delete >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Delete user !WORKING_USER! failed."
        exit /b 1
    )
    
    call :Log "Delete user !WORKING_USER! success."

    exit /b 0

rem %~1:/d delete !WORKING_USER! from the seservicelogonright
:seservicelogonright
    set UNICODE_VAL=
    set SIGNATURE_VAL=
    set REVISION_VAL=
    set SESLOR_VAL=
    set SEDRILOR_VAL=
    set SEDILOR_VAL=
    
    rem export seservicelogonright config
    if exist "!SESERV_LOGON_RIGHT_FILE!"  del /f /q "!SESERV_LOGON_RIGHT_FILE!"
    
    secedit /export /cfg "!INSTALL_TMP_FILE!" >nul 2>&1
	if not "!errorlevel!" == "0" (
        call :Log "Export seservicelogonright config failed."
        if exist "!INSTALL_TMP_FILE!"  del /f /q "!INSTALL_TMP_FILE!"
        exit /b 1
    )

    rem get the info that Log on as a service need
    for /f "delims=" %%a in ('type "!INSTALL_TMP_FILE!"') do (
        set V_KEY=
        for /f "tokens=1 delims==" %%i in ('echo.%%a') do (set V_KEY=%%i)
        set V_KEY=!V_KEY: =!
        
        if "Unicode" == "!V_KEY!" (set UNICODE_VAL=%%a)

        if "signature" == "!V_KEY!" (set SIGNATURE_VAL=%%a)

        if "Revision" == "!V_KEY!" (set REVISION_VAL=%%a)
        
        if "SeServiceLogonRight" == "!V_KEY!" (
            set SESLOR_VAL=%%a,%WORKING_USER%
        )
        
        if "SeDenyRemoteInteractiveLogonRight" == "!V_KEY!" (
            set SEDRILOR_VAL=%%a,%WORKING_USER%
        )
        
        if "SeDenyInteractiveLogonRight" == "!V_KEY!" (
            set SEDILOR_VAL=%%a,%WORKING_USER%
        )
    )
    
    rem the right working user need is empty
	if not "%~1" == "/d" (
        if "!SESLOR_VAL!" == "" (
            set SESLOR_VAL=SeServiceLogonRight = %WORKING_USER%
        )
        if "!SEDRILOR_VAL!" == "" (
            set SEDRILOR_VAL=SeDenyRemoteInteractiveLogonRight = %WORKING_USER%
        )
        if "!SEDILOR_VAL!" == "" (
            set SEDILOR_VAL=SeDenyInteractiveLogonRight = %WORKING_USER%
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
    
    call :Log "get the infos that log on as a service need from the installtmpinfo"
    if exist "!INSTALL_TMP_FILE!"  del /f /q "!INSTALL_TMP_FILE!"
    
    rem import seservicelogonright config to .sdb file
    secedit /import /db "!SESERV_LOGON_RIGHT_TMP_DB!" /cfg "!SESERV_LOGON_RIGHT_FILE!" >nul 2>&1
	if not "!errorlevel!" == "0" (
        call :Log "Import the !SESERV_LOGON_RIGHT_FILE! to the !SESERV_LOGON_RIGHT_TMP_DB! failed."
        if exist "!SESERV_LOGON_RIGHT_FILE!" del /f /q "!SESERV_LOGON_RIGHT_FILE!"
        exit /b 1
    )
    
    if exist "!SESERV_LOGON_RIGHT_FILE!" del /f /q "!SESERV_LOGON_RIGHT_FILE!"
    
    rem add user seservicelogonright Log on as a service
	secedit /configure /db "!SESERV_LOGON_RIGHT_TMP_DB!" >nul 2>&1
	if not "!errorlevel!" == "0" (
        call :Log "Add user !WORKING_USER! seservicelogonright to log on as a service failed."
        if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
        exit /b 1
    )

    if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
    call :Log "Set user !WORKING_USER! seservicelogonright to log on as a service succ."
    exit /b 0	
	
rem ==================regiser to pm end======================
:setipaddress
    echo Obtaining ProtectAgent network adapter information. Please wait...
    call :Log "Obtaining ProtectAgent network adapter information. Please wait..."
    call :Log "Check the EIP can be used to connect to the PM or not."
    if not "%EIP%" == "" (
        for /f "tokens=1,2,3 delims=:" %%i in ('2^>nul ipconfig ^| findstr /i "!IP_FILTER!"') do (
            if " !EIP!" == "%%j" (
                for %%a in (%PMIP_LIST:,= %) do ( 
                    call :CheckIpsConnectivity "%%a" "%PM_PORT%" "!EIP!"
                    if "!errorlevel!" == "0" (
                        echo Set EIP !EIP! successfully!.
                        call :Log "Set EIP !EIP! as host IP successfully!."
                        set host_ip=!EIP!
                        exit /b 0
                    )
                )
            )
        )
    )
    for /f "tokens=1,2,3 delims=:" %%i in ('2^>nul ipconfig ^| findstr /i "!IP_FILTER!"') do (
        if "%%j" NEQ " 127.0.0.1" (
            set host_ip=%%j
            for %%a in (%PMIP_LIST:,= %) do ( 
                call :CheckIpsConnectivity "%%a" "%PM_PORT%" "!host_ip!"
                if "!errorlevel!" == "0" (
                    echo Set host IP !host_ip! successfully!.
                    call :Log "Set host IP !host_ip! successfully!."
                    exit /b 0
                )
            )
        )
    )
    echo No vallid local ip.
    call :Log "No vallid local ip."
    goto :gotoexit
    exit /b 1
     
    
:checkIpsValid
    set ips=%~1
    echo !ips! | findstr "\." > nul && (
        set IP_TYPE=IPV4
    ) || (
        set IP_TYPE=IPV6
    )
    for %%a in (%ips:,= %) do (
        call :CheckIpsConnectivity "%%a" "%PM_PORT%"
        if "!errorlevel!" == "0" (
            exit /b 0
        )
    )
exit /b 1

:setpmip
    rem -------- Confirm Server IP Address --------
    echo !PMIP_LIST! | findstr "\." > nul && (
        echo The server ip is ipv4 [!PMIP_LIST!].
        set IP_TYPE=IPV4
    ) || (
        echo !PMIP_LIST! | findstr "\:" > nul && (
            echo The server ip is ipv6 [!PMIP_LIST!]
            set IP_TYPE=IPV6
        ) || (
            echo Invalid server ip [!PMIP_LIST!].
            call :Log "Invalid server ip [!PMIP_LIST!]"
            goto :gotoexit
			exit /b 1
        )
    )
    set PM_IP=
    for %%a in (%PMIP_LIST:,= %) do (
        call :CheckIpsConnectivity "%%a" "%PM_PORT%"
        if "!errorlevel!" == "0" (
            set PM_IP=!PM_IP!,%%a
        )
    )
    set PM_IP=!PM_IP:~1!
    if "!PM_IP!" == "" (
        echo Check pm ip [!PMIP_LIST!] failed.
        call :Log "Connect to pm ip failed."
        goto :gotoexit
        exit /b 1
    ) else (
        echo Check pm ip [!PM_IP!] success.
        call :Log "Connect to pm ip success."
    )
    exit /b 0

rem ~1: dst_ip, 2: dst_port 3: src_ip
:CheckIpsConnectivity
    call "!AGENT_BIN_PATH!\agentcli.exe" testhost "%~1" "%~2" "5000" %~3
    exit /b !errorlevel!

:setports
        set PORT_NUM=
		
		if "%NUMBER%" LSS "3" (
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
				goto :gotoexit
			    exit /b 1
		    )
        
			set PORT_EXIST=
			set PORT_FLAG=0
			for /f "tokens=1,2,3* delims=: " %%a in ('2^>nul netstat -an ^| findstr !PORT_NUM!') do (
                if "%%c" == "!PORT_NUM!" (
                    if not "%%a" == "UDP" (
                        set PORT_FLAG=1
                    ) else (
                        call :Log "The port !PORT_NUM! is also bound to udp."
                    )
				)
			)
			 
			if !PORT_FLAG! EQU 1 (
				echo The port number !PORT_NUM! is used by other process!
                call :Log "The port number !PORT_NUM! is used by other process!."
				set /a NUMBER+=1
				ping -n 2 127.0>nul
				goto :setports
			)
			
			if "%AGENT_PORT%" == "!PORT_NUM!" (
				echo nginx port number !PORT_NUM! is same with agent port number %AGENT_PORT%.
                call :Log "The nginx port number !PORT_NUM! is same with agent port number %AGENT_PORT%."
				set /a NUMBER+=1
				ping -n 2 127.0>nul
				goto :setports
			)
		) else (
			echo The !PORT_NAME! port you get is error.
			call :Log "The !PORT_NAME! port you get is error."
			goto :gotoexit
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
	    goto :gotoexit
		exit /b 1
    )
    if "!PotrUser!" == "nginx" ( set NGINX_PORT=!PortTemp!)
    if not "!errorlevel!" == "0" ( 
        echo set nginx port failed.
        call :Log "Set nginx port failed."
	    goto :gotoexit
		exit /b 1
    )
	exit /b 0

:registServices
    set SERVICE_NAME=%~1
    set SERVICE_PARAM=%~2
    set REGISTER_USER=
	
    if not "" == "%~3" (set REGISTER_USER=.\%~3)
    
    call :Log "Register Service !SERVICE_NAME! of DataBackup Agent."
    
    sc stop !SERVICE_NAME! >> %LOGFILE_PATH% 2>&1
    
    call "%AGENT_BIN_PATH%\winservice.exe" !SERVICE_PARAM! uninstall
    if not "!errorlevel!" == "0" (
        call :unregisterall
        echo Service !SERVICE_NAME! of DataBackup Agent was uninstalled failed.
        call :Log "Service !SERVICE_NAME! of DataBackup Agent uninstall failed."
        goto :gotoexit
		exit /b 1
    )
    
    if "" == "%~3" (
        call "%AGENT_BIN_PATH%\winservice.exe" !SERVICE_PARAM! install
    ) else (
        call "%AGENT_BIN_PATH%\winservice.exe" !SERVICE_PARAM! install !REGISTER_USER! !WORKING_USER_PWD!
    )
    
    if not "!errorlevel!" == "0" (
        call :unregisterall
        echo Service !SERVICE_NAME! of DataBackup Agent was registered failed.
        call :Log "Register Service !SERVICE_NAME! of DataBackup Agent failed."
        goto :gotoexit
		exit /b 1
    )
    
    call :Log "Service !SERVICE_NAME! of DataBackup Agent was registered successfully."
    echo Service !SERVICE_NAME! of DataBackup Agent was registered successfully.
    
    exit /b 0

:modifyServiceBinPath
    set SERVICE_NAME=%~1
    set NEW_BIN_PATH=%~2

    sc stop !SERVICE_NAME! >> %LOGFILE_PATH% 2>&1
    sc config !SERVICE_NAME! binPath= "!NEW_BIN_PATH!" >> %LOGFILE_PATH% 2>&1
    if not "!errorlevel!" == "0" (
        echo Service !SERVICE_NAME! modify bin path failed.
        call :Log "Service !SERVICE_NAME! modify bin path failed."
        goto :gotoexit
        exit /b 1
    )
exit /b 0

:checkhostip
    if not "%EIP%" == "" (
        echo !EIP! | findstr "\." > nul && (
            set IP_TYPE=IPV4
        ) || (
            set IP_TYPE=IPV6
        )
        call :Log "Checked local ip !EIP! type succeed."
    )
    exit /b 0

:modifyconffile
    set ISPECIL_NUM=0
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "!AGENT_ROOT_PATH!\nginx\conf\nginx.conf"') do (
        for /f %%j in ('2^>nul echo.%%b ^| findstr  !PARAM_NAME!') do (
            set IFLAF_NUM=%%a 
        )
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr  ssl_ciphers') do (
            set ISPECIL_NUM=%%a 
        )
    )

    if !IFLAF_NUM! equ 0 (
        call :Log "Don't find the !PARAM_NAME! in the nginx.conf, exit 1."
        goto :gotoexit
		exit /b 1
    )
    
    if !ISPECIL_NUM! equ 0 (
        call :Log "Don't find the ssl_ciphers in the nginx.conf, exit 1."
        goto :gotoexit
		exit /b 1
    )

    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "!AGENT_ROOT_PATH!\nginx\conf\nginx.conf"') do (
        if %%a equ %IFLAF_NUM% (
            if "!PARAM_NAME!" == "listen" (
                if "!IP_TYPE!" == "IPV4" (
                    echo.        !PARAM_NAME!       !host_ip!:!PORT_NUM! ssl;>>"!AGENT_ROOT_PATH!\tmp\nginx.conf.bak"
                 ) else if "!IP_TYPE!" == "IPV6" (
                    set IP_ADDRESS=[%EIP%]
                    echo.        !PARAM_NAME!       !IP_ADDRESS!:!PORT_NUM! ssl;>>"!AGENT_ROOT_PATH!\tmp\nginx.conf.bak"
                ) else (
                    call :Log "The network Type invalid"
                    goto :gotoexit
					exit /b 1
                )
                
            ) else (
                echo.            !PARAM_NAME!   127.0.0.1:!PORT_NUM!;>>"!AGENT_ROOT_PATH!\tmp\nginx.conf.bak"
            )
        ) else (
            echo.%%b >>"!AGENT_ROOT_PATH!\tmp\nginx.conf.bak"
        )   
    )
    MOVE "!AGENT_ROOT_PATH!\tmp\nginx.conf.bak" "!AGENT_ROOT_PATH!\nginx\conf\nginx.conf" >nul
    
    call :Log "Set %PORT_NAME% port %PORT_NUM% successfully."
    exit /b 0
    
:unregisterall
    if "%UPGRADE_VALUE%" == "1" (
	    echo Agent upgrade failed doesn't need uninstall service.
		call :Log "Agent upgrade failed doesn't need uninstall service."
		goto :gotoexit
		exit /b 1
		
	)
    call "%AGENT_BIN_PATH%\winservice.exe" monitor uninstall
    call "%AGENT_BIN_PATH%\winservice.exe" nginx   uninstall
    call "%AGENT_BIN_PATH%\winservice.exe" rdagent uninstall
    
    exit /b 0

:startservice
    set SERVICE_CHECK=
    set SERVICE_NAME=%~1
    set RETRY_COUNT=1
    
    for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
    if not "!SERVICE_CHECK!" == "" (
    
        echo The service !SERVICE_NAME! of DataBackup Agent is already exist.
        call :Log "The service !SERVICE_NAME! of DataBackup Agent is already exist."
        
    ) else (
    
        sc start !SERVICE_NAME! >>"%LOGFILE_PATH%" 2>&1
        
        :waitstart
            call :WinSleep 3
            
            set SERVICE_CHECK=
            for /f "delims=" %%i in ('2^>nul sc query !SERVICE_NAME! ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
            
            if not "!SERVICE_CHECK!" == "" (
            
                echo The service !SERVICE_NAME! of DataBackup Agent is running now.
                call :Log "The service !SERVICE_NAME! of DataBackup Agent is running now."
                
            ) else (
                rem retry 5 times 
                if !RETRY_COUNT! LEQ 5 (
                    set /a RETRY_COUNT+=1
                    call :Log "Wait the service !SERVICE_NAME! status is stopped, and retry !RETRY_COUNT! times after 3 sec."
                    goto :waitstart
                )
                
                echo The service !SERVICE_NAME! of DataBackup Agent was started failed.
                call :Log "The service !SERVICE_NAME! of DataBackup Agent was started failed after retry 5 times, exit 1."
                call :WinSleep 3
				
                goto :gotoexit
				exit /b 1
            )
			exit /b 0
    )
    exit /b 0
	
:clearsource
    rem ----------delete user rdadmin------------
    if "!UPGRADE_VALUE!" == "1" (
        echo The upgrade process does not require users and groups to be reset.
        exit /b 0
    )
    if /i "!InDomain!"=="TRUE" (
        echo "Agent working user !WORKING_USER! exists before install, no need to clear."
        call :Log "Agent working user !WORKING_USER! exists before install, no need to clear."
    ) else (
        call :deleteworkinguser
        if not "!errorlevel!" == "0" (
            echo Clear source failed.
            call :Log "Clear source failed."
            exit /b 1
        )
    )
    exit /b 0

:WinSleep
    ping 127.0.0.1 -n %~1 > nul
	exit /b 0

:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
    exit /b 0
	
:collectlog
	if exist "%AGENT_ERR_LOG_PATH%" (
	    if exist "!AGENT_LOG_PATH!" (
		    del /f /s /q "%AGENT_ERR_LOG_PATH%\*.*" >nul 2>&1
	        copy /y "!AGENT_LOG_PATH!" "%AGENT_ERR_LOG_PATH%" >nul 2>&1
	    )
	) else (
	    md "%AGENT_ERR_LOG_PATH%" >nul 2>&1
		copy /y "!AGENT_LOG_PATH!" "%AGENT_ERR_LOG_PATH%" >nul 2>&1
	)
	exit /b 0
	
:gotoexit	
	call :clearsource
	call :collectlog

	exit /b 1

:LogError
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_update_prepare_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
	goto :EOF

endlocal