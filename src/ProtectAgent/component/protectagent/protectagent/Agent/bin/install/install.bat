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

rem -----------------------------------------------
rem this shell for an unified installation(window)
rem -----------------------------------------------

cd /d "%~dp0"
set CURRENT_PATH=%~dp0

set DEFAULT_INSTALL_PATH=C:
set CUSTOM_INSTALL_PATH=
set PRECISE_INSTALL_PATH=

rem ---------------------- current download pkg path ------------------------
set AGEN_UPGRADE_BIN_PATH=%CURRENT_PATH%ProtectClient-e\ProtectClient-E\bin
set AGENT_INSTALL_LOG_PATH=%CURRENT_PATH%log
set AGENT_INSTALL_CONF_PATH=%CURRENT_PATH%\conf

rem ---------------------- agent PATH -------------------------------------
set AGENT_PKG_INSTALL_PATH=\DataBackup
set AGENT_MANAGER_PATH=%AGENT_PKG_INSTALL_PATH%\ProtectClient
set AGENT_ROOT_PATH=%AGENT_MANAGER_PATH%\ProtectClient-E
set PLUGIN_DIR=%AGENT_MANAGER_PATH%\Plugins
set AGENT_ERR_LOG_PATH=\var\log\ProtectAgent
set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin
set AGENT_CONF_PATH=%AGENT_ROOT_PATH%\conf
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\log
set REGULAR_IP_FILE=%AGENT_ROOT_PATH%\tmp\ip.info

set LOG_ERR_FILE=%AGENT_ROOT_PATH%\tmp\errormsg.log
set PLUGIN_GENERAL_DB_ENV=%PLUGIN_DIR%\GeneralDBPlugin\bin\applications
set MERGINT_RES_FILE_NAME=%AGENT_ROOT_PATH%\tmp\merging_res.ini

rem ---------------------- Upgrade ------------------------------
set AGENT_UPGRADE_OLD_PATH=%AGENT_PKG_INSTALL_PATH%\AgentUpgrade\Old
set UPGRADE_INITIAL_VALUE=9
set MODIFY_INITIAL_VALUE=9
set /a UPGRADE_VALUE=0

for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set "Current_Time=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"
set AGENT_OLD_LOG_PATH=\var\log\ProtectClient%Current_Time%

rem ------------------------------- other conf --------------------------------
set PLUGINS_DOMAIN_PATH=C:\Windows\System32\drivers\etc\hosts
set WORKING_USER=rdadmin
set AGENT_HOSTSN_PATH=C:\Users\Default
set AGENT_SERVICE_PACKAGE=
set DATATURBO_FUNC=%AGENT_BIN_PATH%\dataturbo_func.bat
set IS_ENABLE_DATATURBO=false

set /a BACKUP_SCENE=0
set /a CPU_NUM_MIN=4

set /a PUSH_INSTALL_VALUE=0
set CMD_PAUSE=pause

set LOG_LEVEL=

rem ------------------------------- limit value -------------------------
set /a FREE_SPACE_MAX=2147483647
set /a FREE_SPACE_MIN=4096


rem -------------error code----------------------
set /a ERR_ADNIMISTRATOR_PER_RETCODE=72
set /a ERR_SERVICE_IS_EXIST=78
set /a ERR_INSTALL_PATH_NOT_EXIST=100
set /a ERR_DISK_FREE_ISLESS_4GB=18
set /a ERR_WORKINGUSER_EXIST=20
set /a ERR_GET_INFO_FAILD=21
set /a ERR_ADAPT_PKG=22
set /a ERR_INSTALL_PLUGINS=23
set /a ERR_PLUGIN_FILE_NOT_EXIST=25
set /a ERR_COPY_LOG=26
set /a ERR_VERIFY_PAS=27
set /a ERR_HOSTDATE_CHECK_FAILD_RETCODE=73
set /a ERR_CERT_NOT_EXIST_RETCODE=74
set /a ERR_AGENT_PATH_IS_EXIST_RETCODE=75
set /a ERR_ADNIMISTRATOR_PER=1577210131
set /a ERR_AGENT_PATH_IS_EXIST=1577210135
set /a ERR_HOSTDATE_CHECK_FAILD=1577210133
set /a ERR_CERT_NOT_EXIST=1577210134
set /a ERR_INSTALL_DATATURBO_RETCODE=21
set /a ERR_INSTALL_DATATURBO=1577210105
set /a ERR_SET_HOSTIP=1577210136
rem -------------error code----------------------

set LOGFILE_PATH=%CURRENT_PATH%install.log
set IP_REX="^[0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*$"

echo ********************************************************
echo      Start the installation of DataBackup ProtectAgent     
echo ********************************************************
rem -----------------adapting upgrade/push process-----------------
if exist %LOGFILE_PATH% (
	del /f /q %LOGFILE_PATH%
)

if "%~1" == "upgrade" (
    set /a UPGRADE_VALUE=1
	set CMD_PAUSE=
)
if "%~1" == "push" (
    set /a PUSH_INSTALL_VALUE=1
	set CMD_PAUSE=
)

rem ------------Checking Administrator Permissions------------
call :Log "Start check the running permission.."
net.exe session 1>NUL 2>NUL && (
    echo Run the bat script as an administrator.
) || (
    echo Not run the bat script as an administrator.
    call :Log "The bat script must run as an administrator."
	%CMD_PAUSE%
	call :clearlogs
	call :archivelogs
    exit /b %ERR_ADNIMISTRATOR_PER_RETCODE%
)

rem ------------Check if agent have been installed------------
rem version number:  1.2  and  1.3
set OLD_VERSION_INSTALL_DIR=C:\OceanProtect\ProtectClient\ProtectClient-E
if exist %OLD_VERSION_INSTALL_DIR% (
    echo "The installation directory %OLD_VERSION_INSTALL_DIR% already exists. Please uninstall it before installation."
    call :Log "The installation directory %OLD_VERSION_INSTALL_DIR% already exists. Please uninstall it before installation."
    call :clearlogs
    call :archivelogs
    call :LogError "The installation directory %OLD_VERSION_INSTALL_DIR% already exists. Please uninstall it before installation." %ERR_AGENT_PATH_IS_EXIST%
    %CMD_PAUSE%
    exit /b %ERR_AGENT_PATH_IS_EXIST_RETCODE%
)
set OLD_VERSION_INSTALL_DIR=C:\DataBackup\ProtectClient\ProtectClient-E
if exist %OLD_VERSION_INSTALL_DIR% (
    echo "The installation directory %OLD_VERSION_INSTALL_DIR% already exists. Please uninstall it before installation."
    call :Log "The installation directory %OLD_VERSION_INSTALL_DIR% already exists. Please uninstall it before installation."
    call :clearlogs
    call :archivelogs
    call :LogError "The installation directory %OLD_VERSION_INSTALL_DIR% already exists. Please uninstall it before installation." %ERR_AGENT_PATH_IS_EXIST%
    %CMD_PAUSE%
    exit /b %ERR_AGENT_PATH_IS_EXIST_RETCODE%
)
rem version number: 1.5
set DATA_BACKUP_AGENT_HOME_VAR=
for /f "tokens=2 delims==" %%a in ('wmic environment where "name='DATA_BACKUP_AGENT_HOME' and username='<system>'" get VariableValue /value') do (
    if not "%%a" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%%a
    )
)
set DATA_BACKUP_AGENT_HOME=
if "%DATA_BACKUP_AGENT_HOME_VAR%" == "" (
    echo DATA_BACKUP_AGENT_HOME not defined
    call :Log "DATA_BACKUP_AGENT_HOME not define."
    set DATA_BACKUP_AGENT_HOME=%DEFAULT_INSTALL_PATH%
) else (
    set DATA_BACKUP_AGENT_HOME=%DATA_BACKUP_AGENT_HOME_VAR%
    echo DATA_BACKUP_AGENT_HOME=%DATA_BACKUP_AGENT_HOME_VAR%
    call :Log "DATA_BACKUP_AGENT_HOME have been define as %DATA_BACKUP_AGENT_HOME_VAR%."
)

set agentRootPath=%DATA_BACKUP_AGENT_HOME%%AGENT_ROOT_PATH%
if exist "%agentRootPath%" (
    echo The installation directory %agentRootPath% already exists. Please uninstall it before installation.
    call :Log "The installation directory %agentRootPath% already exists. Please uninstall it before installation."
    call :clearlogs
    call :archivelogs
    call :LogError "The installation directory %agentRootPath% already exists. Please uninstall it before installation." %ERR_AGENT_PATH_IS_EXIST%
    %CMD_PAUSE%
    exit /b %ERR_AGENT_PATH_IS_EXIST_RETCODE%
)

rem ================check Service exist begin=================
call :Log "Start check Service exist."

call :checkServices rdagent
call :Judgmentoutput !errorlevel! %ERR_SERVICE_IS_EXIST%
if NOT "%errorlevel%" == "0" (
	exit /b %ERR_SERVICE_IS_EXIST%
)

call :checkServices rdnginx
call :Judgmentoutput !errorlevel! %ERR_SERVICE_IS_EXIST%
if NOT "%errorlevel%" == "0" (
	exit /b %ERR_SERVICE_IS_EXIST%
)

call :checkServices rdmonitor
call :Judgmentoutput !errorlevel! %ERR_SERVICE_IS_EXIST%
if NOT "%errorlevel%" == "0" (
	exit /b %ERR_SERVICE_IS_EXIST%
)

rem ---------------------check user-----------------
call :Log "Start checking the install user radmin."
if "!UPGRADE_VALUE!" == "1" (
    echo The upgrade process does not require users and groups to be reset.
    call :Log "The upgrade process does not require users and groups to be reset"
) else (
    net user !WORKING_USER! 1>nul 2>nul
    if "!errorlevel!" == "0" (
        echo Agent working user !WORKING_USER! exist.
        call :Log "Agent working user !WORKING_USER! exist, exit."
		call :gotoexit
		call :clearlogs
		call :archivelogs
		%CMD_PAUSE%
        exit /b %ERR_WORKINGUSER_EXIST%
    )
)

rem ---------------Obtaining Client.conf Information-------------
call :Log "Start parsing the parameter file client.conf."
if exist "!CURRENT_PATH!conf\client.conf" (
    call :ObtainClientConfInfo
) else (
    echo The configuration file client.conf cannot be found.
    call :Log "The configuration file client.conf cannot be found."
    call :gotoexit
	call :clearlogs
	call :archivelogs
	exit /b %ERR_GET_INFO_FAILD%
)

rem ---------------Get package version info -----------------------------
call :Log "Start parsing the package description file package.json."
if exist "!CURRENT_PATH!conf\package.json" (
	set filepath=!CURRENT_PATH!conf\package.json
	for /f "tokens=1,2 delims=:, " %%a in ( 'findstr /l "releaseVersion" "!filepath!"' ) do ( 
		set pkg_version=%%b
	)
	set  PKG_VERSION=!pkg_version:~1,-1!
	for /f "tokens=1,2 delims=:, " %%a in ( 'findstr /l "versionTimeStamp" "!filepath!"' ) do ( 
		set version_time_stamp=%%b
	)
	set VERSION_TIME_STAMP=!version_time_stamp:~1,-1!
) else (
	echo The configuration file package.json cannot be found.
	call :Log "The configuration file package.json cannot be found."
	call :gotoexit
	call :clearlogs
	call :archivelogs
	exit /b %ERR_GET_INFO_FAILD%
)

rem -------------- Adaptation installation package -------------
echo "%PROCESSOR_ARCHITECTURE%" | findstr "64" > nul &&(
    set SYS_BIT=64
)
call :Log "Start adapt the client package."
call :AdaptClientPackage
if %AdaptPackage_Return% EQU 1 (
    echo Not found any suitable package,check whether the name of the installation package is correct.
    call :Log "Not found any suitable package,check whether the name of the installation package is correct."
    call :gotoexit
    call :clearlogs
    call :archivelogs
    exit /b %ERR_ADAPT_PKG%
)
call :Log "Adapt the client package successfully."

rem ----------------- Get custom install path --------------------
call :Log "Start read custom install path."
set /a ATTEMPT_LEFT=3
call :ReadCustomInstallPath
call :Judgmentoutput !errorlevel! !ERR_INSTALL_PATH_NOT_EXIST!
if NOT "%errorlevel%" == "0" (
    echo "Read custom installation path fail."
    call :Log "Read custom installation path fail."
    exit /b %!errorlevel!%
)

if "%CUSTOM_INSTALL_PATH%" == "" (
    set PRECISE_INSTALL_PATH=%DEFAULT_INSTALL_PATH%
) else (
    set PRECISE_INSTALL_PATH=%CUSTOM_INSTALL_PATH%
)
call :Log "Precise installation path is %PRECISE_INSTALL_PATH%."
call :MakeAgentAbsolutePath %PRECISE_INSTALL_PATH%

if exist %AGENT_ROOT_PATH% (
    echo The installation directory %AGENT_ROOT_PATH% already exists. Please uninstall it before installation.
    call :Log "The installation directory %AGENT_ROOT_PATH% already exists. Please uninstall it before installation."
    call :clearlogs
    call :archivelogs
    call :LogError "The installation directory %AGENT_ROOT_PATH% already exists. Please uninstall it before installation." %ERR_AGENT_PATH_IS_EXIST%
    %CMD_PAUSE%
    exit /b %ERR_AGENT_PATH_IS_EXIST_RETCODE%
)

rem -------------------Set environment variable--------------------
setx /M DATA_BACKUP_AGENT_HOME %PRECISE_INSTALL_PATH% >> %LOGFILE_PATH%
set DATA_BACKUP_AGENT_HOME=%PRECISE_INSTALL_PATH% 
echo DATA_BACKUP_AGENT_HOME=%DATA_BACKUP_AGENT_HOME%

rem -----------------check host date-------------------
call :Log "Start check host date."
echo The current host time and time zone are %date% %time%. Check whether the time and time zone are the same as those on DataBackup time:(y/n): 
if "%~1" == "push" (
    echo You have confirmed that the installation continues.
) else (
    if "!UPGRADE_VALUE!" == "1" (
		echo You have confirmed that the installation continues.
	) else (
        set /p key= Your choice:

	    if "!key!" == "n" (
			echo The current system time is not consistent with the DataBackup. As a result, the installation process exits abnormally.
            %CMD_PAUSE%
            call :clearlogs
			call :archivelogs
            call :LogError "The current system time is not consistent with the DataBackup X8000. As a result, the installation process exits abnormally."
            exit /b 1
	    ) else if "!key!" == "y" (
			echo You have confirmed that the installation continues. 
	    ) else (
			echo Please enter y or n.
			%CMD_PAUSE%
			call :clearlogs
			call :archivelogs
			call :LogError "Check host date failed" %ERR_HOSTDATE_CHECK_FAILD%
			exit /b %ERR_HOSTDATE_CHECK_FAILD_RETCODE%
	    )
    )
)

rem --------------------check host resource--------------------
call :Log "Start check host resource."
echo Start check host resource.
md tmp 1>nul 2>nul
call :checkcpunumber
call :checkdiskspace
call :Judgmentoutput !errorlevel! %ERR_DISK_FREE_ISLESS_4GB%
if NOT "%errorlevel%" == "0" (
	exit /b %ERR_DISK_FREE_ISLESS_4GB%
)

rem -------------- copy agent_pkg and decompress -----------------------
if exist "%AGENT_ROOT_PATH%" (
    copy /y "%CURRENT_PATH%ProtectClient-e\!AGENT_SERVICE_PACKAGE!" "%AGENT_ROOT_PATH%" 1>nul 2>nul
    "%CURRENT_PATH%third_party_software\7ZIP\7z.exe" x -tzip -y "%CURRENT_PATH%ProtectClient-e\!AGENT_SERVICE_PACKAGE!" -o"%AGENT_ROOT_PATH%" -mx=9 1>nul 2>nul
) else (
    md %AGENT_ROOT_PATH%
    copy /y "%CURRENT_PATH%ProtectClient-e\!AGENT_SERVICE_PACKAGE!" "%AGENT_ROOT_PATH%" 1>nul 2>nul
    "%CURRENT_PATH%third_party_software\7ZIP\7z.exe" x -tzip -y "%CURRENT_PATH%ProtectClient-e\!AGENT_SERVICE_PACKAGE!" -o"%AGENT_ROOT_PATH%" -mx=9 1>nul 2>nul 
)
del /f "%AGENT_ROOT_PATH%\!AGENT_SERVICE_PACKAGE!" 1>nul 2>nul
if "%LOG_LEVEL%" == "0" (
    call "!AGENT_BIN_PATH!\xmlcfg.exe" write System log_level %LOG_LEVEL%
)

rem -------------- record win version info --------------------
set WIN_RELEASE_VERSION=
call :GetWinReleaseVersion
if not "%WIN_RELEASE_VERSION%" == "" (
    call "!AGENT_BIN_PATH!\xmlcfg.exe" write System win_version %WIN_RELEASE_VERSION%
)

rem -------------- Prepare manage script -----------------------
copy /y "%CURRENT_PATH%start.bat" "%AGENT_MANAGER_PATH%" 1>nul 2>nul
copy /y "%CURRENT_PATH%stop.bat" "%AGENT_MANAGER_PATH%" 1>nul 2>nul
copy /y "%CURRENT_PATH%uninstall.bat" "%AGENT_MANAGER_PATH%" 1>nul 2>nul
copy /y "%CURRENT_PATH%\updateCert.bat" "%AGENT_MANAGER_PATH%" 1>nul 2>nul
copy /y "%CURRENT_PATH%\crl_update.bat" "%AGENT_MANAGER_PATH%" 1>nul 2>nul

rem ----------------------copy cert---------------------
call :Log "Start copy cert."

md %AGENT_ROOT_PATH%\upgrade
copy /y "%CURRENT_PATH%\conf\upgrade_public_key.pem" "%AGENT_ROOT_PATH%\upgrade" 1>nul 2>nul

set NO_EXIST_CERT_FILE=
call :CheckCertFileExist
if not "%NO_EXIST_CERT_FILE%" == "" (
    echo Not found the %NO_EXIST_CERT_FILE% file.
    call :Log "Not found the %NO_EXIST_CERT_FILE% file."
	call :gotoexit
	call :clearlogs
	call :archivelogs
	call :LogError "Not found the %NO_EXIST_CERT_FILE% file." %ERR_CERT_NOT_EXIST%
	exit /b %ERR_CERT_NOT_EXIST_RETCODE%
)

if exist "!AGENT_INSTALL_CONF_PATH!" (
    copy /y "%AGENT_INSTALL_CONF_PATH%\ca.crt.pem" "!AGENT_ROOT_PATH!\nginx\conf\pmca.pem" 1>nul 2>nul
    copy /y "%AGENT_INSTALL_CONF_PATH%\client.crt.pem" "!AGENT_ROOT_PATH!\nginx\conf\server.pem" 1>nul 2>nul
    copy /y "%AGENT_INSTALL_CONF_PATH%\client.pem" "!AGENT_ROOT_PATH!\nginx\conf\server.key" 1>nul 2>nul
	copy /y "%AGENT_INSTALL_CONF_PATH%\agentca.crt.pem" "!AGENT_ROOT_PATH!\nginx\conf\agentca.pem" 1>nul 2>nul
)
call :Log "Copying the certificate is complete."

rem --------------modify pluginmgr.xml----------------
call :Log "Start to modify pluginmgr.xml."
set /a NUMBER=0
for /f "delims=" %%a in ('type "!AGENT_CONF_PATH!\version"') do (
	set /a NUMBER+=1
	if !NUMBER! EQU 2 (
		set AGENT_BUILD_NUM=%%a
	)
)
(
echo ^<^?xml version="1.0" encoding="UTF-8"?^>^ 
echo ^<^Config^>^ 
echo     ^<^PluginList^>^ 
echo         ^<^Plugin name="libhost" version="!AGENT_BUILD_NUM!" service="host" lazyload="0"^>^ ^<^/Plugin^>^ 
echo         ^<^Plugin name="libappprotect" version="!AGENT_BUILD_NUM!" service="tasks" lazyload="0"^>^ ^<^/Plugin^>^ 
echo     ^<^/PluginList^>^ 
echo ^<^/Config^>^ 
)>"!AGENT_CONF_PATH!\pluginmgr.xml"

rem ---------------manual choose is eip-------------------
if NOT "!UPGRADE_VALUE!" == "1" (
    if "!PUSH_INSTALL_VALUE!" == "0" (
        echo  Check whether the host is an EIP node ^(y/n^):
        set /p eip_key= Your choice:
        if "!eip_key!" == "n" (
            echo user choose close datatubo service.
        ) else if "!eip_key!" == "y" (
            set IS_EIP=true
        ) else (
            echo Please enter y or n.
            %CMD_PAUSE%
            call :gotoexit
            call :clearlogs
            call :archivelogs
            exit /b 0
        )
    )
)
call :GetEip
if not "!errorlevel!" == "0" (
	echo Enter eip failed.
	call :LogError "Enter eip failed." %ERR_SET_HOSTIP%
	exit /b %ERR_SET_HOSTIP%
)

rem -------------------write param----------------------
call :WriteParamTmp

rem -------------Read And Verify Passwd---------------
call :Log "Start to obtain the certificate password."
set /a ATTEMPT_LEFT=3
if "%~1" == "upgrade" (
    if exist "%AGENT_UPGRADE_OLD_PATH%" (
	    call :readAndVerifyPasswd upgrade
	) else (
		echo Old Agent backup path doesn't exist.
		call :Log "Old Agent backup path doesn't exist."
		exit /b 1
	)
) else if "%~1" == "push" (
    call :readAndVerifyPasswd push
) else (
    call :readAndVerifyPasswd
)
call :Judgmentoutput !errorlevel! %ERR_VERIFY_PAS%
if NOT "%errorlevel%" == "0" (
    call :Log "Read and verify passwd fail."
	exit /b %ERR_VERIFY_PAS%
)
call :Log "Obtaining the certificate password succeeded."
call :DealDataturbo
call :Judgmentoutput !errorlevel! %ERR_INSTALL_DATATURBO%
if not "!errorlevel!" == "0" (
	echo Install dataturbo failed.
	call :LogError "Install dataturbo failed." %ERR_INSTALL_DATATURBO%
	exit /b %ERR_INSTALL_DATATURBO_RETCODE%
)

rem -----------------install plugins--------------------
call :Log "Start install the plugins."
echo Start install the plugins.
call :InstallPlugins
call :Judgmentoutput !errorlevel! %ERR_INSTALL_PLUGINS%
if NOT "%errorlevel%" == "0" (
	call :LogError "Plugins install error." %ERR_INSTALL_PLUGINS%
	exit /b %ERR_INSTALL_PLUGINS%
)

rem ---------------merge configration files--------------------
if "!UPGRADE_VALUE!" == "1" (
    set OLD_CONF_FILE_PATH_FILEPLUGIN="!OLD_AGENT_BACKUP_PATH!\Plugins\FilePlugin\conf\hcpconf.ini"
    set NEW_CONF_FILE_PATH_FILEPLUGIN="!PLUGIN_DIR!\FilePlugin\conf\hcpconf.ini"
    call :MergeConfFile !OLD_CONF_FILE_PATH_FILEPLUGIN! !NEW_CONF_FILE_PATH_FILEPLUGIN!
    if NOT "%errorlevel%" == "0" (
	    call :LogError "Merge config file %NEW_CONF_FILE_PATH% failed."
    ) else (
        copy /y "!MERGINT_RES_FILE_NAME!" "!NEW_CONF_FILE_PATH_FILEPLUGIN!" 1>nul 2>nul
        del -f -q !MERGINT_RES_FILE_NAME!
    )
)

rem -------------------excute install-----------------
call :Log "Start install the agent."
echo Start install the agent.
if "%~1" == "upgrade" (
    call :ExecuteInstallion upgrade
) else if "%~1" == "push" (
    call :ExecuteInstallion push
) else (
    call :ExecuteInstallion
)
set errRet=!errorlevel!

rd /q %CURRENT_PATH%ProtectClient-e\tmp

%CMD_PAUSE%
exit /b %errRet%

rem ------------------------------Step end-------------------------------

rem %~1 rdagent, rdmonitor, rdnginx   
:checkServices
    call :Log "Check Service %~1."
    set SERVICE_CHECK=
    for /f "delims=" %%i in ('2^>nul sc query %~1 ^| findstr /i "%~1"') do (set SERVICE_CHECK=%%i)
	if "!UPGRADE_VALUE!" == "1" (
	    echo Upgrade process needs service exist.
		call :Log "Upgrade process needs service %~1 exist."
		exit /b 0
	)
    if not "!SERVICE_CHECK!" == "" (
        call :Log "Service %~1 is exist."
        echo Service %~1 is exist.
		exit /b 1
    )
exit /b 0

:MakeAgentAbsolutePath
    set installPath=%~1
    set AGENT_PKG_INSTALL_PATH=%installPath%%AGENT_PKG_INSTALL_PATH%
    set AGENT_MANAGER_PATH=%installPath%%AGENT_MANAGER_PATH%
    set AGENT_ROOT_PATH=%installPath%%AGENT_ROOT_PATH%
    set PLUGIN_DIR=%installPath%%PLUGIN_DIR%
    set AGENT_ERR_LOG_PATH=%installPath%%AGENT_ERR_LOG_PATH%
    set AGENT_OLD_LOG_PATH=%installPath%%AGENT_OLD_LOG_PATH%
    set AGENT_BIN_PATH=%installPath%%AGENT_BIN_PATH%
    set AGENT_CONF_PATH=%installPath%%AGENT_CONF_PATH%
	set AGENT_LOG_PATH=%installPath%%AGENT_LOG_PATH%
    set AGENT_LOG_ERR_MSG_FILE=%installPath%%AGENT_LOG_ERR_MSG_FILE%
    set PLUGIN_GENERAL_DB_ENV=%installPath%%PLUGIN_GENERAL_DB_ENV%

    set AGENT_UPGRADE_OLD_PATH=%installPath%%AGENT_UPGRADE_OLD_PATH%
    set DATATURBO_FUNC=%installPath%%DATATURBO_FUNC%
    set REGULAR_IP_FILE=%installPath%%REGULAR_IP_FILE%

    call :Log "AGENT_ROOT_PATH=%AGENT_ROOT_PATH% have been set"
    exit /b 0

:checkcpunumber
    call :Log "Check cpu number of DataBackup Agent installation."
	
    for /f "tokens=1,2,3* delims==" %%a in ('wmic cpu get NumberOfCores /value') do (
		if "%%a" == "NumberOfCores" (
			set CPUNUM=%%b
		)
	)
	if not "!errorlevel!" == "0" (
	    echo Get cpu number failed.
		call :Log "Get cpu number failed."
		exit /b 0
	) 
    if !CPUNUM! LSS !CPU_NUM_MIN! (
		echo Warning:The installation computer cpu number !CPUNUM! is less than the minimum cpu number requirements !CPU_NUM_MIN!.
		call :Log "Warning:The installation computer cpu number !CPUNUM! is less than the minimum cpu number requirements !CPU_NUM_MIN!."
		exit /b 0
	)
	
	call :Log "Enough cpu number !CPUNUM! of DataBackup Agent installation."
exit /b 0
	
:checkdiskspace
    call :Log "Check free space of DataBackup Agent installation."

    set CURRRENT_DIVER=!AGENT_ROOT_PATH:~0,2!
    for /f "tokens=1,2,3* delims==" %%a in ( 'wmic LogicalDisk where "Caption='!CURRRENT_DIVER!'" get FreeSpace /value' ) do (
	    if "%%a" == "FreeSpace" (
		    set FREE_SPACE=%%b
		)
	)
	
	if "!FREE_SPACE!" GEQ "%FREE_SPACE_MAX%" (
	    call :Log "The installation path free space !FREE_SPACE! is more than the maxmum space requirements !FREE_SPACE_MAX!."
	)
    set FREE_SPACE=!FREE_SPACE:~0,-3!

    rem free space is too large
    if not "!FREE_SPACE!" == "-1" (
        set /a FREE_SPACE=!FREE_SPACE! / 1024
    ) else (
	    echo Get free space failed.
		call :Log "Get free space failed."
		exit /b 1
	)
    if not "!FREE_SPACE!" == "-1" (
        if !FREE_SPACE! LSS !FREE_SPACE_MIN! (
            echo Warning:The installation path free space !FREE_SPACE! is less than the minimum space requirements !FREE_SPACE_MIN!.
            call :Log "Warning:The installation path free space !FREE_SPACE! is less than the minimum space requirements !FREE_SPACE_MIN!."
			exit /b 1
        )
    )
    
    call :Log "Free space !FREE_SPACE! of DataBackup Agent installation."
exit /b 0
	
:AdaptClientPackage
    if exist "%CURRENT_PATH%ProtectClient-e" (
        cd /d "%CURRENT_PATH%ProtectClient-e"
    ) else (
        call :Log "Not found ProtectClient-e file."
        set AdaptPackage_Return=1
        exit /b 1
    )
    set flag=0
    rem -------- traversal file directories --------
    cmd /c "dir *.*/b > local.txt exit" 2>nul
    for /f %%a in (local.txt) do (
		echo "%%a" | findstr "Windows" | findstr "%SYS_BIT%" > nul && (
			set AGENT_SERVICE_PACKAGE=%%a
			set flag=1
		)
	)
    del local.txt
    
    if %flag% EQU 0 (
        call :Log "Adaptation pkg failed."
        set AdaptPackage_Return=1
        exit /b 1
    )
    call :Log "find agent service package %AGENT_SERVICE_PACKAGE%."
    set AdaptPackage_Return=0
exit /b 0

:ObtainClientConfInfo
    if not exist "!CURRENT_PATH!conf\client.conf" (
        call :Log "!CURRENT_PATH!conf\client.conf not exist."
        exit /b 1
    )
    for /f "usebackq tokens=1,2 delims==" %%a in ("!CURRENT_PATH!conf\client.conf") do (
        if "%%a"=="pm_ip" (
            set PM_IP=%%b
        )
        if "%%a"=="pm_manager_ip" (
            set PM_MANAGER_IP=%%b
        )
        if "%%a"=="pm_manager_port" (
            set PM_MANAGER_PORT=%%b
        )
        if "%%a"=="user_id" (
            set USER_ID=%%b
        )
        if "%%a"=="domain" (
            set DOMAIN=%%b
        )
        if "%%a"=="InstallType" (
            set InstallType=%%b
        )
        if "%%a"=="pm_port" (
            set PM_PORT=%%b
        )
        if "%%a"=="is_enable_dataturbo" (
            set IS_ENABLE_DATATURBO=%%b
        )
        if "%%a"=="backup_role" (
            set BACKUP_ROLE=%%b
        )
        if "%%a"=="eip" (
            set EIP=%%b
        )
        if "%%a"=="custom_install_path" (
            set CUSTOM_INSTALL_PATH=%%b
        )
        if "%%a"=="application_info" (
            set APPLICATION_INFO=%%b
        )
        if "%%a"=="private_ip" (
            set PRIVATE_IP=%%b
        )
		if "%%a"=="floating_ip" (
            set FLOATING_IP=%%b
        )
        if "%%a"=="log_level" (
            set LOG_LEVEL=%%b
        )
        if "%%a"=="available_zone" (
            set AVAILABLE_ZONE=%%b
        )
    )
    exit /b 0

:CheckCertFileExist
    if not exist "%AGENT_INSTALL_CONF_PATH%\ca.crt.pem" (
        set NO_EXIST_CERT_FILE=ca.crt.pem
        exit /b 1
    )
    if not exist "%AGENT_INSTALL_CONF_PATH%\agentca.crt.pem" (
        set NO_EXIST_CERT_FILE=agentca.crt.pem
        exit /b 1
    )
    if not exist "%AGENT_INSTALL_CONF_PATH%\client.crt.pem" (
        set NO_EXIST_CERT_FILE=client.crt.pem
        exit /b 1
    )
    if not exist "%AGENT_INSTALL_CONF_PATH%\client.pem" (
        set NO_EXIST_CERT_FILE=client.pem
        exit /b 1
    )
exit /b 0

rem 检查分布式场景
:CheckDistributedScenes
    for /f "tokens=2 delims==" %%a in ('type !CURRENT_PATH!conf\client.conf ^| findstr /c:"deploy_type"') do (
        set DEPLOY_TYPE=%%a
    )
    if "%DEPLOY_TYPE%" == "d7" (
        exit /b 1
    )
    exit /b 0

:DealDataturbo
    call :CheckDistributedScenes
    if not "!errorlevel!" == "0" (
        echo The Distributed Scenes do not support dataturbo.
        call :Log "The Distributed Scenes do not support dataturbo."
        exit /b 0
    )
    rem ---------------choose install datatubo-------------------
    call :Log "choose install datatubo."
    if NOT "!UPGRADE_VALUE!" == "1" (
        if "!PUSH_INSTALL_VALUE!" == "0" (
            echo  whether enable datatubo service ^(y/n^):
            set /p dataturbo_key= Your choice:
            if "!dataturbo_key!" == "n" (
                echo user choose close datatubo service.
            ) else if "!dataturbo_key!" == "y" (
                set IS_ENABLE_DATATURBO=true
                echo user choose open datatubo service.
            ) else (
                echo Please enter y or n.
                %CMD_PAUSE%
                call :gotoexit
                call :clearlogs
                call :archivelogs
                exit /b %ERR_INSTALL_DATATURBO_RETCODE%
            )
        )
    )
    if "%IS_ENABLE_DATATURBO%"=="false" (
        call :Log "service close and not install dataturbo"
        exit /b 0
    ) else if "%IS_ENABLE_DATATURBO%"=="true" (
        call %DATATURBO_FUNC% InstallDataTurbo "%CURRENT_PATH%install.log" "%CURRENT_PATH%" "!MODE!"
	    if NOT "!errorlevel!" == "0" (
		    call :LogError "Install dataturbo failed." %ERR_INSTALL_DATATURBO%
		    exit /b %ERR_INSTALL_DATATURBO_RETCODE%
	    )
    )
exit /b 0

:GetEip
    if "%IS_EIP%"=="true" (
        if !ATTEMPT_LEFT! LEQ 0 (
            echo "You have entered invalid eip(!enterIp!) more than 3 times."
            call :Log "You have entered invalid eip(!enterIp!) more than 3 times."
            exit /b 1
        )
        if !ATTEMPT_LEFT! EQU 3 (
            echo "You need to enter the eip."
            set /p enterIp="Please enter the eip:"
        ) else (
            call :Log "The eip(!enterIp!) is invalid."
            echo "The eip(!enterIp!) is invalid, Please re-enter eip again:"
            set enterIp=
            set /p enterIp=
        )
        echo !enterIp!>"%REGULAR_IP_FILE%"
        set IP_CHECK=
        for /f "delims=" %%i in ('findstr /i %IP_REX% "%REGULAR_IP_FILE%"') do (set IP_CHECK=%%i)
        if exist "%REGULAR_IP_FILE%" (del /f /q "%REGULAR_IP_FILE%")
        if NOT "!IP_CHECK!" == "" (
            echo "You enter eip: !enterIp!."
            call :Log "You enter eip: !enterIp!."
            set EIP=!enterIp!
            exit /b 0
        )
        set /a ATTEMPT_LEFT-=1
        goto :GetEip
        
    )
exit /b 0

:InstallPlugins
	if exist "%CURRENT_PATH%ProtectClient-e\Plugins" (
		cd "%CURRENT_PATH%ProtectClient-e\Plugins"
	) else (
		echo Plugins path is not exist.
		exit /b 0
	)

	for /f "delims=" %%a in ('dir "%CURRENT_PATH%ProtectClient-e\Plugins\*.zip" /b') do (
		set PLIGUN_NAME=%%~na
		set PLUGIN_FILE_NAME=%%a

		md "!PLUGIN_DIR!\!PLIGUN_NAME!" 1>nul 2>nul
        md "%AGENT_ROOT_PATH%\log\Plugins\!PLIGUN_NAME!" 1>nul 2>nul

		copy /y "%CURRENT_PATH%ProtectClient-e\Plugins\!PLUGIN_FILE_NAME!" "!PLUGIN_DIR!\!PLIGUN_NAME!" >nul
		"%CURRENT_PATH%third_party_software\7ZIP\7z.exe" x -tzip -y "%CURRENT_PATH%ProtectClient-e\Plugins\!PLUGIN_FILE_NAME!" -o"!PLUGIN_DIR!\!PLIGUN_NAME!" -mx=9 >nul

        if not exist "!PLUGIN_DIR!\!PLIGUN_NAME!\install.bat" (
            echo "!PLUGIN_DIR!\!PLIGUN_NAME!\install.bat not exist"
            call :Log "!PLUGIN_DIR!\!PLIGUN_NAME!\install.bat not exist."
            exit /b 1
        )

		echo Start to install plugin !PLIGUN_NAME!.
		call :Log "Start to install plugin !PLIGUN_NAME!."
		call "!PLUGIN_DIR!\!PLIGUN_NAME!\install.bat"
		if not "!errorlevel!" == "0" (
			echo Install plugin !PLIGUN_NAME! failed.
			call :Log "Install plugin !PLIGUN_NAME! failed."
			exit /b 1
		)
        echo Install plugin !PLIGUN_NAME! success.
        call :Log "Install plugin !PLIGUN_NAME! success."
	)
	md "%AGENT_CONF_PATH%\PluginPid" 1>nul 2>nul 
	echo Install Plugins successfully.
	call :Log "Install Plugins successfully."

    call :SavePluginsLogs
	
	rem set host ssl domain name
    if "%BACKUP_SCENE%" == "0" (
        for /f "tokens=1-5 delims==" %%a in ('%AGENT_BIN_PATH%\openssl x509 -subject -in %AGENT_ROOT_PATH%\nginx\conf\server.pem -noout') do (
            set DomainName=%%e
        )
        if "!DomainName!" == "" (
            echo Get ssl domain failed.
            call :Log "Get ssl domain failed."
            exit /b 1
        )

        for /f "tokens=1,2* delims= " %%a in ( 'type "%PLUGINS_DOMAIN_PATH%" ^| findstr "!DomainName!"' ) do ( set tmpDomainName=%%b)
        if "!tmpDomainName!" == "" (
			echo:>> "%PLUGINS_DOMAIN_PATH%"
            echo 127.0.0.1 !DomainName! >> "%PLUGINS_DOMAIN_PATH%"
            if not "!errorlevel!" == "0" (
                echo Set ssl domain failed.
                call :Log "Set ssl domain failed."
                exit /b 1
            )
            echo Set ssl domain successfully.
            call :Log "Set ssl domain successfully."
            exit /b 0
        )
        echo Openssl domain has existed.
        call :Log "Openssl domain has existed."
        exit /b 0
    )
exit /b 0

:SavePluginsLogs
    if "%~1" == "upgrade" (
        exit /b 0
    )
    set BACKUP_PLUGINS_LOG_PATH=%AGENT_UPGRADE_OLD_PATH%\ProtectClient-E\log\Plugins
    set CURRENT_PLUGINS_LOG_PATH=%AGENT_ROOT_PATH%\log\Plugins

    if not exist "%CURRENT_PLUGINS_LOG_PATH%" (
        call :Log "The path %CURRENT_PLUGINS_LOG_PATH% not exists."
        exit /b 0
    )
    if not exist "%BACKUP_PLUGINS_LOG_PATH%" (
        call :Log "The path %BACKUP_PLUGINS_LOG_PATH% not exists."
        exit /b 0
    )
    echo Start to save old plugin logs.
    call :Log "Start to save old plugin logs."
    set TMP_PLUGINS_LOG_PATH=%BACKUP_PLUGINS_LOG_PATH%\..\PluginLogTmp
    mkdir "%TMP_PLUGINS_LOG_PATH%"
    xcopy /e/y/q "%BACKUP_PLUGINS_LOG_PATH%" "%TMP_PLUGINS_LOG_PATH%" >nul
    xcopy /e/y/q "%CURRENT_PLUGINS_LOG_PATH%" "%TMP_PLUGINS_LOG_PATH%" >nul
    xcopy /e/y/q "%TMP_PLUGINS_LOG_PATH%" "%CURRENT_PLUGINS_LOG_PATH%" >nul
    rmdir /s/q "%TMP_PLUGINS_LOG_PATH%" >nul 2>&1
exit /b 0

:MergeConfFile
    set OLD_CONF_FILE_PATH=%~1
    set NEW_CONF_FILE_PATH=%~2
    if not exist "!NEW_CONF_FILE_PATH!" (
        call :Log "The file is not exists."
        exit /b 1
    )
    if not exist "!OLD_CONF_FILE_PATH!" (
        call :Log "The file is not exists."
        exit /b 1
    )
    call "!AGENT_BIN_PATH!\agentcli.exe" mergefile !OLD_CONF_FILE_PATH! !NEW_CONF_FILE_PATH!

exit /b 0
	
:ExecuteInstallion
	if "%~1" == "upgrade" ( 
		call "%AGENT_BIN_PATH%\agent_install.bat" upgrade
	) else if "%~1" == "push" (
		cmd /c echo !password! ^|"%AGENT_BIN_PATH%\agent_install.bat" push
	) else ( 
		call "%AGENT_BIN_PATH%\agent_install.bat"
	)
	if not "!errorlevel!" == "0" (
	    set ERR_EXCUT=!errorlevel!
        echo DataBackup Agent was installed failed.
        call :Log "DataBackup Agent was installed failed."
        exit /b !ERR_EXCUT!
    )

    copy /y "%LOGFILE_PATH%"  "%AGENT_ROOT_PATH%\log" >nul
    del /q /f "%LOGFILE_PATH%" >nul

    echo The DataBackup ProtectAgent has been installed successfully.
    call :Log "The DataBackup ProtectAgent has been installed successfully."
exit /b 0

:GetWinReleaseVersion
    set WIN_VERSION=

    rem cmd ver: Microsoft Windows [Version 10.0.22000.2057]
    for /f "tokens=1* delims=[" %%a in ('ver') do (
        set WIN_VERSION=%%b
    )
    call :Log "WIN_VERSION=%WIN_VERSION%"

    rem WIN_VERSION=Version 10.0.22000.2057]
    set WIN_VERSION=%WIN_VERSION:~0,-1%

    rem WIN_VERSION=Version 10.0.22000.2057
    for /f "tokens=1,2" %%a in ("%WIN_VERSION%") do (
        set WIN_VERSION=%%b
    )

    rem WIN_VERSION=10.0.22000.2057
    set BIG=
    set SMALL=
    for /f "tokens=1,2 delims=." %%a in ("%WIN_VERSION%") do (
        set BIG=%%a
        set SMALL=%%b
    )
    rem BIG=10 SMALL=0
    set WIN_RELEASE_VERSION=%BIG%.%SMALL%
    call :Log "WIN_RELEASE_VERSION=%WIN_RELEASE_VERSION%"
exit /b 0

:WriteParamTmp
	set PARAMS_PATH=%AGENT_CONF_PATH%
    (echo SYS_BIT=!SYS_BIT!) > "!PARAMS_PATH!\testcfg.tmp"
    (echo InstallType=!InstallType!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo PM_IP=!PM_IP!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo PM_PORT=!PM_PORT!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo PM_MANAGER_IP=!PM_MANAGER_IP!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo IS_ENABLE_DATATURBO=!IS_ENABLE_DATATURBO!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo PM_MANAGER_PORT=!PM_MANAGER_PORT!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo USER=!WORKING_USER!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo USER_ID=!USER_ID!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo DOMAIN_NAME=!DOMAIN!) >> "!PARAMS_PATH!\testcfg.tmp" 
    (echo PKG_VERSION=!PKG_VERSION!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo VERSION_TIME_STAMP=!VERSION_TIME_STAMP!) >> "!PARAMS_PATH!\testcfg.tmp" 
    (echo UPGRADE_STATUS=!UPGRADE_INITIAL_VALUE!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo BACKUP_ROLE=!BACKUP_ROLE!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo BACKUP_SCENE=!BACKUP_SCENE!) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo CERTIFICATE_REVOCATION=0) >> "!PARAMS_PATH!\testcfg.tmp"
    (echo MODIFY_STATUS=!MODIFY_INITIAL_VALUE!) >> "!PARAMS_PATH!\testcfg.tmp"
	(echo APPLICATION_INFO=!APPLICATION_INFO!) >> "!PARAMS_PATH!\testcfg.tmp"
    if not "!AVAILABLE_ZONE!" == "" (
		(echo AVAILABLE_ZONE=!AVAILABLE_ZONE!) >> "!PARAMS_PATH!\testcfg.tmp"
	)
	if not "!UPGRADE_VALUE!" == "1" (
		if not "!PRIVATE_IP!" == "" (
			(echo PRIVATE_IP=!PRIVATE_IP!) >> "!PARAMS_PATH!\testcfg.tmp"
		)
		if not "!FLOATING_IP!" == "" (
			(echo FLOATING_IP=!FLOATING_IP!) >> "!PARAMS_PATH!\testcfg.tmp"
		)
	) else (
		if "!PRIVATE_IP!" == "" (
			call :getParamVal "!OLD_AGENT_BACKUP_PATH!\ProtectClient-E\conf\testcfg.tmp" "PRIVATE_IP" "PRIVATE_IP"
		)
		if not "!PRIVATE_IP!" == "" (
			(echo PRIVATE_IP=!PRIVATE_IP!) >> "!PARAMS_PATH!\testcfg.tmp"
		)
		if "!FLOATING_IP!" == "" (
			call :getParamVal "!OLD_AGENT_BACKUP_PATH!\ProtectClient-E\conf\testcfg.tmp" "FLOATING_IP" "FLOATING_IP"
		)
		if not "!FLOATING_IP!" == "" (
			(echo FLOATING_IP=!FLOATING_IP!) >> "!PARAMS_PATH!\testcfg.tmp"
		)
	)
    if "%EIP%" == "" (
        if "!UPGRADE_VALUE!" == "1" (
            if exist "!AGENT_UPGRADE_OLD_PATH!\ProtectClient-E\conf\testcfg.tmp" (
                for /f "usebackq tokens=1,2 delims== " %%a in ("!AGENT_UPGRADE_OLD_PATH!\ProtectClient-E\conf\testcfg.tmp") do (
                    if "%%a"=="EIP" (
                        set EIP=%%b
                        echo EIP=!EIP!>> "!PARAMS_PATH!\testcfg.tmp"
                    )
                )
            )
        ) else (
            call :Log "No eip exist"
        )
    ) else (
        echo EIP=!EIP!>> "!PARAMS_PATH!\testcfg.tmp"
        call "!AGENT_BIN_PATH!\xmlcfg.exe" write Mount eip !EIP!
    )    
exit /b 0

:ReadCustomInstallPath
    if "!PUSH_INSTALL_VALUE!" == "1" (
        rem ---already read custom client.conf---
		call :Log "Push install need read client.conf and get install path."
        echo "Push install set install_path is %CUSTOM_INSTALL_PATH%"
	) else if "!UPGRADE_VALUE!" == "1" (
        set CUSTOM_INSTALL_PATH=%DATA_BACKUP_AGENT_HOME%
        call :Log "Upgrade use origin install path %CUSTOM_INSTALL_PATH%."
    ) else (
        call :Log "Manual installation need read input path."
        call :ReadPathLoop
        if not "!errorlevel!" == "0" (
            exit /b 1
        )
    )
    call :Log "Read custom install path success."
exit /b 0

:ReadPathLoop
    if !ATTEMPT_LEFT! LEQ 0 (
        echo "You have entered invalid installation path(%iPath%) more than 3 times."
        call :Log "You have entered invalid installation path(%iPath%) more than 3 times."
        exit /b 1
    )
    if !ATTEMPT_LEFT! EQU 3 (
        echo "You need to enter the installation path (directly press 'Enter' use default installation path C:)."
        set /p iPath="Please enter custom install path:"
    ) else (
        call :Log "The entered path(%iPath%) does not exist."
        echo "The entered path(%iPath%) does not exist, Please reenter installation path:"
        set iPath=
        set /p iPath=
    )
    if not defined iPath (
        echo "You directly press 'Enter', use default install path C:"
        call :Log "You directly press 'Enter', use default install path C:"
        exit /b 0
    )
    if exist %iPath% (
        echo "You enter installation path: %iPath%."
        call :Log "You enter installation path: %iPath%."
        set CUSTOM_INSTALL_PATH=%iPath%
        exit /b 0
    )
    set /a ATTEMPT_LEFT-=1
    goto :ReadPathLoop

:getParamVal
    set confFile=%~1
	set paramKey=%~2
	set receiveVar=%~3
	if not exist "!confFile!" (
		call :Log "Config file not found"
		exit /b 1
    )
    for /f "usebackq tokens=1,2 delims== " %%a in ("!confFile!") do (
		if "%%a" == "!paramKey!" (
			set !receiveVar!=%%b
		)
	)
exit /b 0

:readAndVerifyPasswd
    if not exist "!AGENT_ROOT_PATH!\tmp" (
        md "!AGENT_ROOT_PATH!\tmp"
    )
	if !ATTEMPT_LEFT! GTR 0 (
		echo Please enter the private key password set on ProtectManager, you still have !ATTEMPT_LEFT! chances:
		call :Log "Please enter the private key password set on ProtectManager, you still have !ATTEMPT_LEFT! chances:"
        if "%~1" == "push" (
		    set /p password=
		    call "!AGENT_BIN_PATH!\agentcli.exe" enckey cipherFile Input !password!
			for /f "delims=" %%a in ('type !AGENT_ROOT_PATH!\tmp\input_tmpcipherFile') do set encrypted_password=%%a
			del /f /q "%AGENT_ROOT_PATH%\tmp\input_tmpcipherFile"
		) else if "%~1" == "upgrade" (
			for /f %%a in ('call "%AGENT_UPGRADE_OLD_PATH%\ProtectClient-E\bin\xmlcfg.exe" read Monitor nginx "ssl_key_password"') do set encrypted_password=%%a
		) else (
		    call "!AGENT_BIN_PATH!\agentcli.exe" enckey cipherFile
		    for /f "delims=" %%a in ('type !AGENT_ROOT_PATH!\tmp\input_tmpcipherFile') do set encrypted_password=%%a
			del /f /q "%AGENT_ROOT_PATH%\tmp\input_tmpcipherFile"
		)
		call "!AGENT_BIN_PATH!\xmlcfg.exe" write Monitor nginx ssl_key_password !encrypted_password!
		set verifykey_result=
		for /f "delims=" %%a in ('call "!AGENT_BIN_PATH!\agentcli.exe" verifykey') do set verifykey_result=%%a
		if "!verifykey_result!" == "failed" (
			echo Failed to verify the private key password.
			call :Log "Failed to verify the private key password."
			set /a ATTEMPT_LEFT-=1
			goto :readAndVerifyPasswd
		)
		echo The private key password is verified successfully.
	    call :Log "The private key password is verified successfully."
		exit /b 0
	)
	echo Failed to verify the private key password for three times.
	call :Log "Failed to verify the private key password for three times."
exit /b 1

:clearsource
    if "%PRECISE_INSTALL_PATH%" == "" (
        echo No need to clear source.
        exit /b 0
    ) else (
        echo PRECISE_INSTALL_PATH=%PRECISE_INSTALL_PATH%
    )
	if "!CLEAR_NUM!" LEQ "4" (
		if exist "%AGENT_ROOT_PATH%" (
			call :WinSleep 10
			rmdir /s /q "%AGENT_ROOT_PATH%" "%PLUGIN_DIR%" 1>nul 2>nul
			set /a CLEAR_NUM+=1
			call :clearsource
		)
		call :Log "Agent install path doesn't exist."
        echo Clear source successfully.
        call :Log "Clear source successfully."
		exit /b 0
	) else (
		echo Clear dir 3 times failed.
		call :Log "Clear dir %AGENT_ROOT_PATH% 3 times failed."
		exit /b 1
	)

:WinSleep
    ping 127.0.0.1 -n %~1> nul
    exit /b 0

:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%CURRENT_PATH%install.log"
    exit /b 0

:Judgmentoutput
    if not "%~1" == "0" (
	    call :gotoexit
		call :archivelogs
		%CMD_PAUSE%
		exit /b %~2
	)
	exit /b 0

:archivelogs
    if exist "%LOGFILE_PATH%" (
	    if exist "%AGENT_ERR_LOG_PATH%" (
		    copy /y "%LOGFILE_PATH%" "%AGENT_ERR_LOG_PATH%" >nul
		)
	    md "%AGENT_ERR_LOG_PATH%" 1>nul 2>nul
		copy /y "%LOGFILE_PATH%" "%AGENT_ERR_LOG_PATH%" >nul
	)
	if not "!errorlevel!" == "0" (
	    echo Copy error log failed.
		exit /b %ERR_COPY_LOG%
	)
    del /q /f "%LOGFILE_PATH%" 1>nul 2>nul
	exit /b 0

:clearlogs
    if exist "%AGENT_ERR_LOG_PATH%" (
        for /f %%i in ('dir /b "%AGENT_ERR_LOG_PATH%"') do set /a count+=1
        if !count! gtr 0 (
            xcopy /e/h/k/x/o/q/y "%AGENT_ERR_LOG_PATH%\*" "%AGENT_OLD_LOG_PATH%\" > nul
            del /f /s /q "%AGENT_ERR_LOG_PATH%\*.*" >nul 2>&1
        )
    )
	exit /b 0

:gotoexit
    echo Failed to install the ProtectAgent. Clearing installation resources. Please wait...
    call :Log "Failed to install the ProtectAgent. Clearing installation resources. Please wait..."
    if NOT "!UPGRADE_VALUE!" == "1" (
		call %DATATURBO_FUNC% UninstallDataTurbo "%CURRENT_PATH%install.log" "%CURRENT_PATH%" "!MODE!"
	)
	set /a CLEAR_NUM =1
	call :clearsource
	if not "!errorlevel!" == "0" (
	    echo Clear source failed.
		call :Log "Clear source failed."
		exit /b 1
 	)
    exit /b 1

:LogError
    if "%PRECISE_INSTALL_PATH%" == "" (
        set LOG_ERR_FILE=%CURRENT_PATH%\errormsg.log
    )
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_update_prepare_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
    goto :EOF

endlocal
