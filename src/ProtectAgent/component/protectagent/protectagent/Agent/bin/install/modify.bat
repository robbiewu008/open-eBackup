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
rem #  %~1:modify type
rem #
rem #  Usage:
rem #         common modify : modify.bat
rem #         push modify : modify.bat /push or modify.bat /P
rem #
rem ########################################################################

setlocal EnableDelayedExpansion

rem Set common path
set CURRENT_PATH=%~dp0
set AGENT_PACKAGE_PATH=%CURRENT_PATH%\ProtectClient-e\
set AGENT_NEWPKG_TMP_PATH=%AGENT_PACKAGE_PATH%\ProtectClient-E
set LOGFILE_PATH=%CURRENT_PATH%\modify.log
set WIN_SYSTEM_DISK=%WINDIR:~0,1%
set DEFAULT_INSTALL_PATH=C:
if not "%WIN_SYSTEM_DISK%" == "" (
    set DEFAULT_INSTALL_PATH=%WIN_SYSTEM_DISK%:
)
set DATA_BACKUP_AGENT_HOME_VAR=
for /f "tokens=2 delims==" %%a in ('wmic environment where "name='DATA_BACKUP_AGENT_HOME' and username='<system>'" get VariableValue /value') do (
    if not "%%a" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%%a
    )
)
if "%DATA_BACKUP_AGENT_HOME_VAR%" == "" (
	set DATA_BACKUP_AGENT_HOME_VAR=%DEFAULT_INSTALL_PATH%
    echo "DATA_BACKUP_AGENT_HOME is not defined, use default path %DEFAULT_INSTALL_PATH%."
)

set OLD_INSTALL_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\ProtectClient
set AGENT_ROOT_PATH=%OLD_INSTALL_PATH%\ProtectClient-E
set AGENT_PLUGIN_PATH=%OLD_INSTALL_PATH%\Plugins
set NEW_INSTALL_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\ProtectClient
set AGENT_PUSHMODIFY_PACKAGE_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\PushModify\
set LOG_ERR_FILE=%NEW_INSTALL_PATH%\ProtectClient-E\tmp\errormsg.log
set PLUGINS_DOMAIN_PATH=C:\Windows\System32\drivers\etc\hosts
if not "%WIN_SYSTEM_DISK%" == "" (
    set PLUGINS_DOMAIN_PATH=%WIN_SYSTEM_DISK%:\Windows\System32\drivers\etc\hosts
)
set BACKUP_PLUGIN_PATH=%DATA_BACKUP_AGENT_HOME_VAR%\DataBackup\BackupPlugin

set MODIFY_TYPE=%~1
set SYS_BIT=
set ADAPT_PACKAGE_NAME=
set NEW_VERSION=
set OLD_VERSION=
set IS_CLEAN=0
set IS_SUCC=0

if exist "%LOGFILE_PATH%" (
    del "%LOGFILE_PATH%"
)

rem -------------error code----------------------

set /a ERR_EXEC_REGISTER=1577210142
set /a ERR_EXEC_REGISTER_RETCODE=82
set /a ERR_MODIFY_PKG_VERSION_LOW=1577210139
set /a ERR_MODIFY_PKG_VERSION_LOW_RETCODE=79
set /a ERR_INSTALL_PLUGIN_RETCODE=71
set /a EER_AGENT_UNINSTALL_FAILED=24
set /a ERR_REGISTER_TO_PM_RET_CODE=37
rem -------------error code----------------------

rem #################### Main Process ##########################
echo ####################Modify begin##########################
call :Log "####################Modify begin##########################"


echo "step1: Check the validity of input parameters"
call :Log "CheckParam function."
call :CheckParam
if NOT %errorlevel% EQU 0 (
    echo "step1: failed"
    call :Log "Exec CheckParam function failed."
    call :ExecExit %ERR_MODIFY_FAIL_VERIFICATE_PARAMETER%
)
call :Log "CheckParam function succ."
echo "step1: succ"

echo "step2: Check whether DataBackup ProtectAgent is installed"
call :Log "CheckInstalled function."
call :CheckInstalled
if NOT %errorlevel% EQU 0 (
    echo "step2: failed"
    call :Log "Exec CheckInstalled function failed."
    call :LogError "Exec CheckInstalled function failed." %ERR_AGENT_PATH_IS_EXIST%
    call :ExecExit %ERR_AGENT_PATH_IS_EXIST_RETCODE%
)
call :Log " CheckInstalled function succ."
echo "step2: succ"

echo "step3: Check current systembit"
call :Log " GetSystemBit function."
call :GetSystemBit
if NOT %errorlevel% EQU 0 (
    echo "step3: failed"
    call :Log "Exec GetSystemBit function failed."
    call :ExecExit %ERR_MODIFY_PACKAGE_NO_MATCH_HOST_SYSTEM%
)
call :Log " GetSystemBit function succ."
echo "step3: succ"

echo "step4: Choose AdaptPackage"
call :Log " AdaptPackage function."
call :AdaptPackage
if NOT %errorlevel% EQU 0 (
    echo "step4: failed"
    call :Log "Exec AdaptPackage function failed."
    call :ExecExit %ERR_UNZIP_PKG%
)
call :Log " AdaptPackage function succ."
echo "step4: succ"

echo "step5: Check DataBackup ProtectAgent Version"
call :Log " CheckVersion function."
call :CheckVersion
if NOT %errorlevel% EQU 0 (
    echo "step5: failed"
    call :Log "Exec CheckVersion function failed."
    call :LogError "Exec CheckVersion function failed." %ERR_MODIFY_PKG_VERSION_LOW%
    call :ExecExit %ERR_MODIFY_PKG_VERSION_LOW_RETCODE%
)
call :Log " CheckVersion function succ."
echo "step5: succ"

echo "Backup old agent plugin"
call :Log "BackupInstalled function."
call :BackupInstalled
if NOT %errorlevel% EQU 0 (
    echo "step5: failed"
    call :Log "Exec CheckVersion function failed."
    call :LogError "Exec CheckVersion function failed." %ERR_MODIFY_PKG_VERSION_LOW%
    call :ExecExit %ERR_MODIFY_PKG_VERSION_LOW_RETCODE%
)
call :Log " BackupInstalled function succ."
echo "BackupInstalled succ"

echo "step6: Stop DataBackup ProtectAgent"
call :Log " StopAgent function."
call :StopAgent
if NOT %errorlevel% EQU 0 (
    echo "step6: failed"
    call :Log "Exec StopAgent function failed."
    call :ExecExit %ERR_MODIFY_FAIL_STOP_PROTECT_AGENT%
)
call :Log " StopAgent function succ."
echo "step6: succ"

echo "step7: uninstall old plugins"
call :UninstAllOldPlugins
if NOT %errorlevel% EQU 0 (
	echo "step7: uninstall old plugin failed"
    call :RollBack
    if NOT !errorlevel! EQU 0 (
        echo "step7.1: Exec UninstAllOldPlugins function, and exec RollBack funciton failed"
        call :Log "Exec uninstall.bat failed, and exec RollBack funciton failed"
        call :ExecExit %EER_AGENT_UNINSTALL_FAILED%
        exit /b 1
    ) else (
        call :Log "Exec  UninstAllOldPlugins function failed, and exec RollBack funciton succeed"
        call :ExecExit  %ERR_REGISTER_TO_PM_RET_CODE%
        exit /b 2
    )
)
echo "step7: succ"

echo "step8: install new plugins"
call :InstallNewPlugins
if NOT %errorlevel% EQU 0 (
	echo "step8: InstallNewPlugins failed"
    call :Log "Exec InstallNewPlugins function failed."
    call :RollBack
    if NOT !errorlevel! EQU 0 (
        echo "step8.1: Exec InstallNewPlugins function failed, and exec RollBack funciton failed"
        call :Log "Exec InstallNewPlugins function failed, and exec RollBack funciton failed"
        call :ExecExit %ERR_EXEC_REGISTER_RETCODE%
        exit /b 1
    ) else (
        echo "step8.2: Exec RollBack funciton succeed"
        call :Log "Exec InstallNewPlugins failed, and exec RollBack funciton succeed"
        call :ExecExit  %ERR_REGISTER_TO_PM_RET_CODE%
        exit /b 2
    )
)

call :RepalceBackupConf

echo "step8: succ"

echo "step9: install new plugins"
call :StartAgent
if NOT %errorlevel% EQU 0 (
	echo "step9: failed"
    call :Log "Exec StartAgent function failed."
    call :LogError "Exec StartAgent function failed." %ERR_EXEC_REGISTER%
    call :ExecExit %ERR_EXEC_REGISTER_RETCODE%
)
echo "step9: succ"

echo "step10: modify application info"
call :ModifyApplicationInfo
if NOT %errorlevel% EQU 0 (
	echo "step10: failed"
    call :Log "Exec ModifyApplicationInfo function failed."
    call :LogError "Exec ModifyApplicationInfo function failed." %ERR_EXEC_REGISTER%
    call :ExecExit %ERR_EXEC_REGISTER_RETCODE%
)
echo "step10: succ"

echo "step11: register agent"
call "%OLD_INSTALL_PATH%\ProtectClient-E\bin\agentcli.exe" registerHost RegisterHost
if NOT %errorlevel% EQU 0 (
	echo "step10: failed"
    call :Log "Exec RegisterAgent function failed."
    call :LogError "Exec RegisterAgent function failed." %ERR_EXEC_REGISTER%
    call :ExecExit %ERR_EXEC_REGISTER_RETCODE%
)
echo "step11: succ"

set IS_SUCC=1
call :ExecExit 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
goto :EOF

:CheckParam
    if not "%MODIFY_TYPE%" EQU "" (
        if not "%MODIFY_TYPE%" EQU "/P" (
            if not "%MODIFY_TYPE%" EQU "/push" (
                echo "    Parameter detection failed, please confirm the parameters."
                call :Log "Parameter detection failed, please confirm the parameters."
                call :PrintUsage
                exit /b 1
            )
        )
        set MODIFY_TYPE=1
        echo "    Current type is push modify."
        call :Log "Current type is push modify, and MODIFY_TYPE is 1."
        exit /b 0
    )
    set MODIFY_TYPE=0
    echo "    Current type is common modify."
    call :Log "Current type is common modify, and MODIFY_TYPE is 0."
    exit /b 0
goto :EOF

:CheckInstalled
    if not exist "%OLD_INSTALL_PATH%\ProtectClient-E" (
        echo "    Not install the DataBackup ProtectAgent."
        call :Log "Not install the DataBackup ProtectAgent, failed to modify."
        exit /b 1
    )
    exit /b 0
goto :EOF

:GetSystemBit
    echo %PROCESSOR_ARCHITECTURE% | findstr "64" > nul && (
        set SYS_BIT=x64
        echo "    The current system is x64."
        call :Log "The current system is x64."
        exit /b 0        
    ) || (
        echo "    The current system architecture is not adapted."
        call :Log "The current system architecture is not adapted."
        exit /b 1
    )
goto :EOF

:AdaptPackage
    set Adapt_file_num=0
    if exist "%AGENT_PACKAGE_PATH%" (
        for /f %%a in ('dir "%AGENT_PACKAGE_PATH%" /b') do (
            echo %%a | findstr /i "Windows" | findstr /i "%SYS_BIT%" | findstr ".zip" > nul && (
                set ADAPT_PACKAGE_NAME=%%a
                set /a Adapt_file_num=!Adapt_file_num! + 1
            )
        )
    ) else (
        echo "    Can't find installation package path."
        call :Log "Not found the Installation package path of the DataBackup ProtectAgent."
        exit /b 1
    )

    if not %Adapt_file_num% EQU 1 (
        echo "    The modify package is not unique."
        call :Log "The number of installation packages is not 1."
        exit /b 1
    )
    call :Log "Modify using package is: %ADAPT_PACKAGE_NAME%."
    if exist "%AGENT_NEWPKG_TMP_PATH%" (
        rd /s /q "%AGENT_NEWPKG_TMP_PATH%"
    )
    md "%AGENT_NEWPKG_TMP_PATH%"
    set IS_CLEAN=1

    if exist "%CURRENT_PATH%\third_party_software\7ZIP\7z.exe" (
        "%CURRENT_PATH%\third_party_software\7ZIP\7z.exe" x -tzip -y "%AGENT_PACKAGE_PATH%%ADAPT_PACKAGE_NAME%"  -o"%AGENT_NEWPKG_TMP_PATH%" >nul
    ) else (
        echo "    Can't find 7z.exe."
        call :Log "Can't find 7z.exe, the modify package is damaged."
    )
    exit /b 0
goto :EOF

:CheckVersion
    if exist "%AGENT_NEWPKG_TMP_PATH%\conf\version" (
        for /f "tokens=1,2 delims=:" %%a in ('findstr /n ".*" "%AGENT_NEWPKG_TMP_PATH%\conf\version"') do (
            if %%a EQU 3 (
                set NEW_VERSION=%%b
                echo !NEW_VERSION!| findstr "^[0-9]*$" > nul || (
                    echo "    Current Version timestamp have invail characters."
                    call :Log "Current Version timestamp have invail characters."
                    exit /b 1
                )
            )
        )
        if "!NEW_VERSION!" EQU "" (
            echo "    Current version timestamp is null."
            call :Log "Current version timestamp is null."
            exit /b 1
        )
    ) else (
        echo "    The modify package version file does not exist."
        call :Log "The modify package version file does not exist."
        exit /b 1
    )
    

    if exist "%OLD_INSTALL_PATH%\ProtectClient-E\conf\version" (
        for /f "tokens=1,2 delims=:" %%a in ('findstr /n ".*" "%OLD_INSTALL_PATH%\ProtectClient-E\conf\version"') do (
            if %%a EQU 3 (
                set OLD_VERSION=%%b
                echo !OLD_VERSION!| findstr "^[0-9]*$" > nul || (
                    echo "    Installed Version timestamp have invail characters."
                    call :Log "Installed Version timestamp have invail characters."
                    exit /b 1
                )
            )
        )
        if "!OLD_VERSION!" EQU "" (
            echo "    Installed version timestamp is null."
            call :Log "Installed version timestamp is null."
            exit /b 1
        )
    ) else (
        echo "    The installed version file does not exist."
        call :Log "The installed version file does not exist"
        exit /b 1
    )

    echo "    Current version is %OLD_VERSION%, and modify package version is %NEW_VERSION%."
    call :Log "Current version is %OLD_VERSION%, and modify package version is %NEW_VERSION%."
    if "%NEW_VERSION%" LSS "%OLD_VERSION%" (
        echo "    Version comparison failed"
        call :Log "    Version comparison failed"
        exit /b 1
    )
    exit /b 0
goto :EOF

:StopAgent
    echo "    Stop DataBackup ProtectAgent."
    call :Log "Call agent_stop.bat, "
    call "%OLD_INSTALL_PATH%\ProtectClient-E\bin\process_stop.bat"
    if NOT %errorlevel% EQU 0 (
        echo "    Stop DataBackup ProtectAgent failed."
        call :Log "Exec agent_stop.bat failed."
        exit /b 1
    )
    exit /b 0
goto :EOF

:StartAgent
    echo "Start DataBackup ProtectAgent."
    call :Log "Call agent_start.bat, "
    call "%OLD_INSTALL_PATH%\ProtectClient-E\bin\agent_start.bat"
    if NOT %errorlevel% EQU 0 (
        echo "    Start DataBackup ProtectAgent failed."
        call :Log "Exec agent_start.bat failed."
        exit /b 1
    )
    exit /b 0
goto :EOF

:CopyLogFile
    call :Log "Copy log file to ProtectClient-E\log\"

    if exist "%LOGFILE_PATH%\agent_modify.log" (
	   copy /y "%LOGFILE_PATH%" "%NEW_INSTALL_PATH%\ProtectClient-E\log" >nul
    )

    call :Log "########Modify finish########"

    if exist "%CURRENT_PATH%\modify.log" (
    	copy /y "%CURRENT_PATH%\modify.log" "%NEW_INSTALL_PATH%\ProtectClient-E\log" >nul
    )
    if NOT exist "%NEW_INSTALL_PATH%\ProtectClient-E\log\modify.log" (
        call :Log "Copy modify.log failed."
        echo "Collect modify.log failed"
    )
goto :EOF

:RepalceBackupConf
    echo Begin to replace the backup DataBackup ProtectAgent Plugin conf.
    call :Log "Modify: Begin to replace the backup DataBackup ProtectAgent Plugin conf."
    set tmpPath=!cd!
    cd !BACKUP_PLUGIN_PATH!

    for /f "delims=" %%d in ('dir /b /ad ^| findstr /i "Plugin"') do (
        set oldPlugin=%%d
        set oldPluginExistNow=
        for /f "delims=" %%e in ('dir /b /ad "!AGENT_PLUGIN_PATH!" ^| findstr /i "%%d"') do (
            set oldPluginExistNow=%%e
        )
        if "!oldPluginExistNow!"=="" (
            call :Log "%%d not used now."
        ) else (
            call :Log "Replace the %%d conf file."
            xcopy /y "!BACKUP_PLUGIN_PATH!\%%d\plugin_attribute_*.json" "!AGENT_PLUGIN_PATH!\%%d\"
            xcopy /y /e "!BACKUP_PLUGIN_PATH!\%%d\conf" "!AGENT_PLUGIN_PATH!\%%d\conf"
        )
    )

    cd !tmpPath!
    call :Log "Modify: The backup DataBackup ProtectAgent Plugin conf has been replaced."
    echo The backup DataBackup ProtectAgent Plugin conf has been replaced successfully.
goto :EOF

:UninstAllOldPlugins
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
	rd /s /q "%AGENT_PLUGIN_PATH%" >nul
	mkdir "%AGENT_PLUGIN_PATH%"
    exit /b 0
	
:InstallNewPlugins
	if exist "%CURRENT_PATH%ProtectClient-e\Plugins" (
		cd "%CURRENT_PATH%ProtectClient-e\Plugins"
	) else (
		echo Plugins path is not exist.
		exit /b 0
	)

	for /f "delims=" %%a in ('dir "%CURRENT_PATH%ProtectClient-e\Plugins" /b') do (
		set PLIGUN_NAME=%%~na
		set PLUGIN_FILE_NAME=%%a
		md "!AGENT_PLUGIN_PATH!\!PLIGUN_NAME!" 1>nul 2>nul
		copy /y "%CURRENT_PATH%ProtectClient-e\Plugins\!PLUGIN_FILE_NAME!" "!AGENT_PLUGIN_PATH!\!PLIGUN_NAME!" >nul
		"%CURRENT_PATH%third_party_software\7ZIP\7z.exe" x -tzip -y "%CURRENT_PATH%ProtectClient-e\Plugins\!PLUGIN_FILE_NAME!" -o"!AGENT_PLUGIN_PATH!\!PLIGUN_NAME!" -mx=9 >nul
		md "%AGENT_ROOT_PATH%\log\Plugins\!PLIGUN_NAME!" 1>nul 2>nul
	
		echo Start to install plugin !PLIGUN_NAME!.
		call :Log "Start to install plugin !PLIGUN_NAME!."
		call "!AGENT_PLUGIN_PATH!\!PLIGUN_NAME!\install.bat"
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

:ModifyApplicationInfo
	for /f "usebackq tokens=1,2 delims==" %%a in ("!CURRENT_PATH!conf\client.conf") do (
        if "%%a"=="application_info" (
            set APP_INFO=%%b
        )
    )
	set AGENT_RUNNING_CONFIG_FILE=%OLD_INSTALL_PATH%\ProtectClient-E\conf\testcfg.tmp
	set AGENT_TMP_RUNNING_CONFIG_FILE=%OLD_INSTALL_PATH%\ProtectClient-E\conf\tmptestcfg.txt
	
	for /f "delims== tokens=1,2*" %%a in ('type "%AGENT_RUNNING_CONFIG_FILE%"') do (
        if "%%a" EQU "APPLICATION_INFO" (
            echo %%a=!APP_INFO!>> "%AGENT_TMP_RUNNING_CONFIG_FILE%"
        ) else (
            echo %%a=%%b>> "%AGENT_TMP_RUNNING_CONFIG_FILE%"
        )
    )
	move /y "%AGENT_TMP_RUNNING_CONFIG_FILE%" "%AGENT_RUNNING_CONFIG_FILE%" >nul
goto :EOF


:PrintUsage
    echo  Usage:
    echo      common modify: modify.bat
    echo      push modify: modify.bat /P or modify.bat /push
    exit /b 0
goto :EOF

:ClearSource
    cd "%CURRENT_PATH%"
    call :Log "Exec ClearSource function, delete new package temporary decompress path"
    rd /s /q "%AGENT_NEWPKG_TMP_PATH%" >nul
goto :EOF

:BackupInstalled
    rem backup installed Agent
    if exist "%BACKUP_PLUGIN_PATH%" (
        rd /s /q  "%BACKUP_PLUGIN_PATH%" > nul
    )
    md "%BACKUP_PLUGIN_PATH%"
    xcopy /e/h/k/x/o/q/y "%AGENT_PLUGIN_PATH%\*" "%BACKUP_PLUGIN_PATH%" > nul 
    if NOT %errorlevel% EQU 0 (
        call :Log "Backup installed Agent plugin failed."
        exit /b 1
    )
    call :Log "Backup installed Agent plugin successful"
    exit /b 0
goto :EOF

:RollBack
    rem restore Agent
    call :Log "Restore installed plugin, in RollBack."
    if exist "%AGENT_PLUGIN_PATH%" (
        rd /s /q  "%AGENT_PLUGIN_PATH%" > nul
    )
    md "%AGENT_PLUGIN_PATH%"
    xcopy /e/h/k/x/o/q/y "%BACKUP_PLUGIN_PATH%\*" "%AGENT_PLUGIN_PATH%" > nul
    echo Y | Cacls "%IN_USE_AGENT_MANAGER_PATH%" /T /E /R Users >nul 2>&1

    rem start Agent
    call :Log "Exec agent_start.bat, in RollBack."
    call :StartAgent
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec agent_start.bat failed"
        exit /b 1
    )

    rem Register Agent to PM 
    call "%OLD_INSTALL_PATH%\ProtectClient-E\bin\agentcli.exe" registerHost RegisterHost
    if NOT %errorlevel% EQU 0 (
        echo "Exec RollBack funciton, failed to registerHost"
        call :Log "Exec RollBack funciton, registerAgent to PM failed."
        call :LogError "Exec RollBack funciton, registerAgent to PM failed." %ERR_EXEC_REGISTER%
        call :ExecExit %ERR_EXEC_REGISTER_RETCODE%
    )
    exit /b 0
goto :EOF

:UpdateModifyStatus
    if %IS_SUCC% EQU 1 (
        set AGENT_RUNNING_CONFIG_FILE=%NEW_INSTALL_PATH%\ProtectClient-E\conf\testcfg.tmp
        set AGENT_TMP_RUNNING_CONFIG_FILE=%NEW_INSTALL_PATH%\ProtectClient-E\conf\tmptestcfg.txt
    ) else (
        set AGENT_RUNNING_CONFIG_FILE=%OLD_INSTALL_PATH%\ProtectClient-E\conf\testcfg.tmp
        set AGENT_TMP_RUNNING_CONFIG_FILE=%OLD_INSTALL_PATH%\ProtectClient-E\conf\tmptestcfg.txt
    ) 
    if exist "%AGENT_TMP_RUNNING_CONFIG_FILE%" (
        del "%AGENT_TMP_RUNNING_CONFIG_FILE%"
    )
    for /f "delims== tokens=1,2*" %%a in ('type "%AGENT_RUNNING_CONFIG_FILE%"') do (
        if "%%a" EQU "MODIFY_STATUS" (
            echo %%a=!IS_SUCC!>> "%AGENT_TMP_RUNNING_CONFIG_FILE%"
        ) else (
            echo %%a=%%b>> "%AGENT_TMP_RUNNING_CONFIG_FILE%"
        )
    )
    move /y "%AGENT_TMP_RUNNING_CONFIG_FILE%" "%AGENT_RUNNING_CONFIG_FILE%" >nul

    call :Log "Backup push modify log, and push modify process ends."
    exit /b 0
goto :EOF

:RemoveInstallPack
    if exist "%BACKUP_PLUGIN_PATH%" (
        rd /s /q "%BACKUP_PLUGIN_PATH%"
    )
    if exist "%AGENT_PUSHMODIFY_PACKAGE_PATH%" (
        rd /s /q "%AGENT_PUSHMODIFY_PACKAGE_PATH%"
    )
    call :Log "Exec RemoveInstallPack function, delete modify package"
    if exist "%AGENT_NEWPKG_TMP_PATH%" (
        rd /s /q "%AGENT_NEWPKG_TMP_PATH%"
    )
    call :Log "Exec RemoveInstallPack function, delete modify package"
goto :EOF

:LogError
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_modify_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
goto :EOF

:ExecExit
    call :Log "modify.bat exec ExecExit"
    if %IS_CLEAN% EQU 1 (
        call :ClearSource
    )
    if %MODIFY_TYPE% EQU 1 (
        call :UpdateModifyStatus
        if %IS_SUCC% EQU 1 (
            cd "%NEW_INSTALL_PATH%"
            call :CopyLogFile
            call :RemoveInstallPack
        ) else (
            cd "%OLD_INSTALL_PATH%"
        )
        endlocal
        exit %1
    )
    call :CopyLogFile
    if %IS_SUCC% EQU 1 (
        echo ####################Modify succeed##########################
        call :Log "####################Modify succeed##########################"
        call :RemoveInstallPack
    ) else (
        echo ####################Modify failed##########################
        call :Log "####################Modify failed##########################"
    )
    call :Log "Exit modify.bat"
    echo "The process is complete, press any key to exit"
    pause
    endlocal
exit 0