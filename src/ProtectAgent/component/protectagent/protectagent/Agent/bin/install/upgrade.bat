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
rem #  %~1:upgrade type
rem #
rem #  Usage:
rem #         common upgrade : upgrade.bat
rem #         push upgrade : upgrade.bat /push or upgrade.bat /P
rem #
rem ########################################################################

setlocal EnableDelayedExpansion

rem ----------------- current new package path -------------------------------
set CURRENT_PATH=%~dp0
set AGENT_NEWPKG_PLACE_PATH=%CURRENT_PATH%
set AGENT_NEWPKG_MANAGER_PATH=%CURRENT_PATH%\ProtectClient-e
set AGENT_NEWPKG_ROOT_PATH=%AGENT_NEWPKG_MANAGER_PATH%\ProtectClient-E
set AGENT_NEWPKG_BIN_PATH=%AGENT_NEWPKG_ROOT_PATH%\bin
set LOGFILE_PATH=%CURRENT_PATH%\upgrade.log

rem ---------------- agent pkg installation relative path ------------------------
set AGENT_PKG_INSTALL_PATH=\DataBackup
set AGENT_MANAGER_PATH=%AGENT_PKG_INSTALL_PATH%\ProtectClient
set AGENT_ROOT_PATH=%AGENT_MANAGER_PATH%\ProtectClient-E
set PLUGIN_DIR=%AGENT_MANAGER_PATH%\Plugins

set AGENT_PUSHUPGRADE_PACKAGE_PATH=%AGENT_PKG_INSTALL_PATH%\PushUpgrade\
set AGENT_PUSHUPGRADE_LOG_PATH=%AGENT_PKG_INSTALL_PATH%\PushUpgrade_Log\
set LOG_ERR_FILE=%AGENT_ROOT_PATH%\tmp\errormsg.log
set DATATURBO_FUNC="%AGENT_NEWPKG_ROOT_PATH%\bin\dataturbo_func.bat"

rem ---------------- current used package install path --------------------------
set WIN_SYSTEM_DISK=%WINDIR:~0,1%
set AGENT_OCEAN_PROTECT_VERSION_ROOT_PATH=C:\OceanProtect\ProtectClient\ProtectClient-E
if not "%WIN_SYSTEM_DISK%" == "" (
    set AGENT_OCEAN_PROTECT_VERSION_ROOT_PATH=%WIN_SYSTEM_DISK%:\OceanProtect\ProtectClient\ProtectClient-E
)
set IN_USE_AGENT_ROOT_PATH=
set IN_USE_AGENT_MANAGER_PATH=

rem ---------------- other info ----------------------------------
set UPGRADDE_TYPE=%~1
set SYS_BIT=
set ADAPT_PACKAGE_NAME=
set NEW_VERSION=
set OLD_VERSION=
set IS_CLEAN=0
set IS_SUCC=0

set DEFAULT_INSTALL_PATH=C:
if not "%WIN_SYSTEM_DISK%" == "" (
    set DEFAULT_INSTALL_PATH=%WIN_SYSTEM_DISK%:
)
set PRECISE_INSTALL_PATH=
set AGENT_IS_STOP=0

rem -------------error code----------------------

set /a ERR_ADNIMISTRATOR_PER=1577210131
set /a ERR_AGENT_PATH_IS_EXIST=1577210135
set /a ERR_SERVICE_IS_EXIST=78
set /a ERR_DISK_FREE_ISLESS_4GB=1677931361
set /a ERR_HOSTDATE_CHECK_FAILD=1577210133
set /a ERR_WORKINGUSER_EXIST=1577209885
set /a ERR_GET_INFO_FAILD=1577209882
set /a ERR_ADAPT_PKG=1677931365
set /a ERR_INSTALL_PLUGINS=1577210130
set /a ERR_CERT_NOT_EXIST=1577210134
set /a ERR_PLUGIN_FILE_NOT_EXIST=25
set /a ERR_COPY_LOG=26
set /a ERR_VERIFY_PAS=1577209890
set /a ERR_INSTALL_PLUGIN_RETCODE=71
set /a ERR_EXEC_REGISTER=1577210142
set /a ERR_EXEC_REGISTER_RETCODE=82
set /a ERR_UPGRADE_PKG_VERSION_LOW=1577210139
set /a ERR_UPGRADE_PKG_VERSION_LOW_RETCODE=79
set /a ERR_UPGRADE_DATATURBO_INSTALL=1577210105
set /a ERR_UPGRADE_DATATURBO_UPGRADE=1577210106
rem -------------error code----------------------

rem #################### Main Process ##########################
echo ####################Upgrade begin##########################
call :Log "####################Upgrade begin##########################"

if exist "%LOGFILE_PATH%" (
    del "%LOGFILE_PATH%"
)

rem --------------------------- Check the validity of input parameters -------------------------------
echo "step1: Check the validity of input parameters"
call :Log "Call CheckParam function."
call :CheckParam
if NOT %errorlevel% EQU 0 (
    echo "step1: failed"
    call :Log "Exec CheckParam function failed."
    call :ExecExit %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
)
call :Log "Call CheckParam function succ."
echo "step1: succ"

rem -------------------------- Check Installation path ----------------------------------
echo "step2: Check whether DataBackup ProtectAgent is installed"
call :Log "Call CheckInstalled function."

set DATA_BACKUP_AGENT_HOME_VAR=
for /f "tokens=2 delims==" %%a in ('wmic environment where "name='DATA_BACKUP_AGENT_HOME' and username='<system>'" get VariableValue /value') do (
    if not "%%a" == "" (
        set DATA_BACKUP_AGENT_HOME_VAR=%%a
    )
)
if not "%DATA_BACKUP_AGENT_HOME_VAR%" == "" (
    set PRECISE_INSTALL_PATH=%DATA_BACKUP_AGENT_HOME_VAR%
    echo "PRECISE_INSTALL_PATH=%PRECISE_INSTALL_PATH%."
) else (
    set PRECISE_INSTALL_PATH=%DEFAULT_INSTALL_PATH%
    echo "DATA_BACKUP_AGENT_HOME is not defined, use default path %DEFAULT_INSTALL_PATH%."
)
call :Log "PRECISE_INSTALL_PATH=%PRECISE_INSTALL_PATH%"
call :MakeAgentAbsolutePath %PRECISE_INSTALL_PATH%

if exist "%AGENT_ROOT_PATH%" (
    set IN_USE_AGENT_ROOT_PATH=%AGENT_ROOT_PATH%
) else if exist "%AGENT_OCEAN_PROTECT_VERSION_ROOT_PATH%" (
    set IN_USE_AGENT_ROOT_PATH=%AGENT_OCEAN_PROTECT_VERSION_ROOT_PATH%
) else (
    echo "Installation path %AGENT_ROOT_PATH% not exist, can not upgrade."
    call :Log "Installation path %AGENT_ROOT_PATH% not exist, can not upgrade."
    call :ExecExit %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
)
set IN_USE_AGENT_MANAGER_PATH=%IN_USE_AGENT_ROOT_PATH%\..
call :Log "Current DataBackup ProtectAgent is installed on the path %IN_USE_AGENT_ROOT_PATH%."
echo "step2: succ"

rem -------------------------- Check current system bit ----------------------------------
echo "step3: Check current systembit"
call :Log "Call GetSystemBit function."
call :GetSystemBit
if NOT %errorlevel% EQU 0 (
    echo "step3: failed"
    call :Log "Exec GetSystemBit function failed."
    call :ExecExit %ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM%
)
call :Log "Call GetSystemBit function succ."
echo "step3: succ" 

rem -------------------------- copy agent_pkg and decompress ----------------------------------
echo "step4: Choose AdaptPackage"
call :Log "Call AdaptPackage function."
call :AdaptPackage
if NOT %errorlevel% EQU 0 (
    echo "step4: failed"
    call :Log "Exec AdaptPackage function failed."
    call :ExecExit %ERR_UNZIP_PKG%
)
call :Log "Call AdaptPackage function succ."
echo "step4: succ"

rem -------------------------- Compare package version old LEQ new ----------------------------------
echo "step5: Check DataBackup ProtectAgent Version"
call :Log "Call CheckVersion function."
call :CheckVersion
if NOT %errorlevel% EQU 0 (
    echo "step5: failed"
    call :Log "Exec CheckVersion function failed."
    call :LogError "Exec CheckVersion function failed." %ERR_UPGRADE_PKG_VERSION_LOW%
    call :ExecExit %ERR_UPGRADE_PKG_VERSION_LOW_RETCODE%
)
call :Log "Call CheckVersion function succ."
echo "step5: succ"

rem -------------------------- Stop ProtectAgent ----------------------------------
echo "step6: Stop DataBackup ProtectAgent"
call :Log "Call StopAgent function."
call :StopAgent
if NOT %errorlevel% EQU 0 (
    echo "step6: failed"
    call :Log "Exec StopAgent function failed."
    call :ExecExit %ERR_UPGRADE_FAIL_STOP_PROTECT_AGENT%
)
call :Log "Call StopAgent function succ."
echo "step6: succ"

rem -------------------------- Upgrade ----------------------------------
echo "step7_pre: Upgrade dataturbo"
set MODE=
if %UPGRADDE_TYPE% EQU 1 (
    set MODE=push
)
call %DATATURBO_FUNC% CheckDataTurboInstalled "%LOGFILE_PATH%" "%CURRENT_PATH%" "!MODE!" >> %LOGFILE_PATH% 2>&1
if !errorlevel! EQU 0 (
    call %DATATURBO_FUNC% UpgradeDataTurbo "%LOGFILE_PATH%" "%CURRENT_PATH%" "!MODE!" >> %LOGFILE_PATH% 2>&1
    if NOT !errorlevel! EQU 0 (
        echo "step7_pre: failed"
        call :Log "Exec UpgradeDataTurbo function failed."
        call :LogError "Exec UpgradeDataTurbo function failed." %ERR_UPGRADE_DATATURBO_UPGRADE%
        call :ExecExit %ERR_UPGRADE_DATATURBO_UPGRADE%
    )
)
echo "step7_pre: upgrade dataturob succ"

echo "step7: Execute the upgrade process"
call :Log "Call UpgradeAgent function."
call :UpgradeAgent
if NOT %errorlevel% EQU 0 (
    echo "step7: failed"
    call :Log "Exec UpgradeAgent function failed."
    call :LogError "Exec UpgradeAgent function failed." %ERR_EXEC_REGISTER%
    call :ExecExit %ERR_EXEC_REGISTER_RETCODE%
)
call :Log "Call UpgradeAgent function succ."
echo "step7: upgrade agent succ"

set IS_SUCC=1
call :ExecExit 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
goto :EOF

:MakeAgentAbsolutePath
    set installPath=%~1
    set AGENT_PKG_INSTALL_PATH=%installPath%%AGENT_PKG_INSTALL_PATH%
    set AGENT_MANAGER_PATH=%installPath%%AGENT_MANAGER_PATH%
    set AGENT_ROOT_PATH=%installPath%%AGENT_ROOT_PATH%

    set AGENT_PUSHUPGRADE_PACKAGE_PATH=%installPath%%AGENT_PUSHUPGRADE_PACKAGE_PATH%
    set AGENT_PUSHUPGRADE_LOG_PATH=%installPath%%AGENT_PUSHUPGRADE_LOG_PATH%
    set LOG_ERR_FILE=%installPath%%LOG_ERR_FILE%

    set PLUGIN_DIR=%installPath%%PLUGIN_DIR%
    exit /b 0

:CheckParam
    call :Log "UPGRADDE_TYPE=%UPGRADDE_TYPE%."
    if not "%UPGRADDE_TYPE%" EQU "" (
        if not "%UPGRADDE_TYPE%" EQU "/P" (
            if not "%UPGRADDE_TYPE%" EQU "/push" (
                echo "    Parameter detection failed, please confirm the parameters."
                call :Log "Parameter detection failed, please confirm the parameters."
                call :PrintUsage
                exit /b 1
            )
        )
        set UPGRADDE_TYPE=1
        echo "    Current type is push upgrade."
        call :Log "Current type is push upgrade, and UPGRADDE_TYPE is 1."
        exit /b 0
    )
    set UPGRADDE_TYPE=0
    echo "    Current type is common upgrade."
    call :Log "Current type is common upgrade, and UPGRADDE_TYPE is 0."
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
    if exist "%AGENT_NEWPKG_MANAGER_PATH%" (
        for /f %%a in ('dir "%AGENT_NEWPKG_MANAGER_PATH%" /b') do (
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
        echo "    The upgrade package is not unique."
        call :Log "The number of installation packages is not 1."
        exit /b 1
    )

    call :Log "Upgrade using package is: %ADAPT_PACKAGE_NAME%."
    if exist "%AGENT_NEWPKG_ROOT_PATH%" (
        rd /s /q "%AGENT_NEWPKG_ROOT_PATH%"
    )
    md "%AGENT_NEWPKG_ROOT_PATH%"
    set IS_CLEAN=1

    if exist "%CURRENT_PATH%\third_party_software\7ZIP\7z.exe" (
        "%CURRENT_PATH%\third_party_software\7ZIP\7z.exe" x -tzip -y "%AGENT_NEWPKG_MANAGER_PATH%\%ADAPT_PACKAGE_NAME%"  -o"%AGENT_NEWPKG_ROOT_PATH%"  >> %LOGFILE_PATH% 2>&1
    ) else (
        echo "    Can't find 7z.exe."
        call :Log "Can't find 7z.exe, the upgrade package is damaged."
    )
    exit /b 0
goto :EOF

:CheckVersion
    if exist "%AGENT_NEWPKG_ROOT_PATH%\conf\version" (
        for /f "tokens=1,2 delims=:" %%a in ('findstr /n ".*" "%AGENT_NEWPKG_ROOT_PATH%\conf\version"') do (
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
        echo "    The upgrade package version file does not exist."
        call :Log "The upgrade package version file does not exist."
        exit /b 1
    )

    if exist "%IN_USE_AGENT_ROOT_PATH%\conf\version" (
        for /f "tokens=1,2 delims=:" %%a in ('findstr /n ".*" "%IN_USE_AGENT_ROOT_PATH%\conf\version"') do (
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

    echo "    Current version is %OLD_VERSION%, and upgrade package version is %NEW_VERSION%."
    call :Log "Current version is %OLD_VERSION%, and upgrade package version is %NEW_VERSION%."
    if "%NEW_VERSION%" LEQ "%OLD_VERSION%" (
        echo "    Version comparison failed"
        call :Log "    Version comparison failed"
        exit /b 1
    )
    exit /b 0
goto :EOF

:StopAgent
    call :Log "Begin to stop agent."
    if %UPGRADDE_TYPE% EQU 0 (
        echo Please choose whether to stop DataBackup ProtectAgent now. ^(y/n^):
        set /p IsStop_flag=">>"
        if not "!IsStop_flag!" EQU "y" (
            echo The current selection is no. Please confirm selection again. ^(y/n^):
            set /p Confirm_flag=">>"
            if not "!Confirm_flag!" EQU "y" (
                echo "    User does not want to stop DataBackup ProtectAgent."
                call :Log "User does not want to stop DataBackup ProtectAgent."
                exit /b 1
            )
        )
    )
    echo "    Stop DataBackup ProtectAgent."
    call :Log "Call agent_stop.bat, "
    call "%IN_USE_AGENT_ROOT_PATH%\bin\process_stop.bat"
    if NOT %errorlevel% EQU 0 (
        echo "    Stop DataBackup ProtectAgent failed."
        call :Log "Exec agent_stop.bat failed."
        exit /b 1
    )
    set AGENT_IS_STOP=1
    exit /b 0
goto :EOF

:ReadDataTurboChoose
    for /f "usebackq tokens=1,2 delims==" %%a in ("!CURRENT_PATH!conf\client.conf") do (
        if "%%a"=="is_enable_dataturbo" (
            set IS_ENABLE_DATATURBO=%%b     
        )
    )
exit /b 0

:DealDataturbo
    if "%IS_ENABLE_DATATURBO%"=="false" (
        echo "service close and not install dataturbo"
        exit /b 0
    ) else if "%IS_ENABLE_DATATURBO%"=="true" (
        call :OpenServiceAndInstallDataTurbo
    )
exit /b 0

:OpenServiceAndInstallDataTurbo
    call %DATATURBO_FUNC% InstallDataTurbo "%CURRENT_PATH%install.log" "%CURRENT_PATH%" "!MODE!" >> %LOGFILE_PATH% 2>&1
	call :Judgmentoutput !errorlevel! %ERR_INSTALL_DATATURBO%
	if NOT "!errorlevel!" == "0" (
		call :LogError "Install dataturbo failed." %ERR_INSTALL_DATATURBO%
		exit /b %ERR_INSTALL_DATATURBO_RETCODE%
exit /b 0

:UpgradeAgent
    if %UPGRADDE_TYPE% EQU 0 (
        call :Log "Call agent_upgrade.bat, mode is commmon."
        call "%AGENT_NEWPKG_ROOT_PATH%\bin\agent_upgrade.bat" %CURRENT_PATH%
    ) else if %UPGRADDE_TYPE% EQU 1 (
        call :Log "Call agent_upgrade.bat, mode is push."
        call "%AGENT_NEWPKG_ROOT_PATH%\bin\agent_upgrade.bat" %CURRENT_PATH% %UPGRADDE_TYPE%
    )
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec agent_upgrade.bat failed."
        if %errorlevel% EQU 2 (
            call :Log "agent_upgarde.bat exec RollBack succ, please read detail in agent_upgrade.log"
        )
        exit /b 1
    )
    exit /b 0
goto :EOF

rem merge file(%~1) from srcDir(%~2) to dstDir(%~3)
:MergeUpgradeLog
    call :Log "merge file(%~1) from srcDir(%~2) to dstDir(%~3)."
    set fileName=%~1
    set srcDir=%~2
    set dstDir=%~3
    if not exist "%srcDir%\%fileName%" (
        call :Log "log file %srcDir%\%fileName% not exist."
        exit /b 1
    )
    if exist "%dstDir%\%fileName%" (
        type %srcDir%\%fileName% >> %dstDir%\%fileName%
    ) else (
        copy /y  %srcDir%\%fileName%  %dstDir%\%fileName% > nul
    )
goto :EOF

:CopyLogFile
    call :Log "Copy upgrade log file to ProtectClient-E\log\"

    call :MergeUpgradeLog agent_upgrade_sqlite.log  %AGENT_NEWPKG_PLACE_PATH%  %AGENT_ROOT_PATH%\log

    call :MergeUpgradeLog agent_upgrade.log  %AGENT_NEWPKG_PLACE_PATH%  %AGENT_ROOT_PATH%\log

    call :MergeUpgradeLog upgrade.log  %AGENT_NEWPKG_PLACE_PATH%  %AGENT_ROOT_PATH%\log

    call :MergeUpgradeLog agent_robocopy.log  %AGENT_NEWPKG_PLACE_PATH%  %AGENT_ROOT_PATH%\log

    call :MergeUpgradeLog install.log %AGENT_NEWPKG_PLACE_PATH%  %AGENT_ROOT_PATH%\log
 
    call :MergeUpgradeLog uninstall.log %AGENT_NEWPKG_PLACE_PATH%  %AGENT_ROOT_PATH%\log

    set LOGFILE_PATH=%AGENT_ROOT_PATH%\log\upgrade.log
goto :EOF

:PrintUsage
    echo  Usage:
    echo      common upgrade: upgrade.bat
    echo      push upgrade: upgrade.bat /P or upgrade.bat /push
    exit /b 0
goto :EOF

:ClearSource
    cd "%CURRENT_PATH%"
    call :Log "Exec ClearSource function, delete new package temporary decompress path"
    rd /s /q "%AGENT_NEWPKG_ROOT_PATH%"  >> %LOGFILE_PATH% 2>&1
goto :EOF

:TrueMove 
    if exist "%~1" (
        call :WinSleep 1
        call :Log "Wait for 1s during file moving."
        move /y "%~1" "%~2"  >> %LOGFILE_PATH% 2>&1
        goto TrueMove
    )
    exit /b 0

:WinSleep
    timeout %1  >> %LOGFILE_PATH% 2>&1
goto :eof

:UpdateUpgradeStatus
    if %IS_SUCC% EQU 1 (
        set AGENT_RUNNING_CONFIG_FILE=%AGENT_ROOT_PATH%\conf\testcfg.tmp
        set AGENT_TMP_RUNNING_CONFIG_FILE=%AGENT_ROOT_PATH%\conf\tmptestcfg.txt
    ) else (
        set AGENT_RUNNING_CONFIG_FILE=%IN_USE_AGENT_ROOT_PATH%\conf\testcfg.tmp
        set AGENT_TMP_RUNNING_CONFIG_FILE=%IN_USE_AGENT_ROOT_PATH%\conf\tmptestcfg.txt
    ) 
    if exist "%AGENT_TMP_RUNNING_CONFIG_FILE%" (
        del "%AGENT_TMP_RUNNING_CONFIG_FILE%"
    )
    for /f "delims== tokens=1,2*" %%a in ('type "%AGENT_RUNNING_CONFIG_FILE%"') do (
        if "%%a" EQU "UPGRADE_STATUS" (
            echo %%a=!IS_SUCC!>> "%AGENT_TMP_RUNNING_CONFIG_FILE%"
        ) else (
            echo %%a=%%b>> "%AGENT_TMP_RUNNING_CONFIG_FILE%"
        )
    )
    call :TrueMove "%AGENT_TMP_RUNNING_CONFIG_FILE%" "%AGENT_RUNNING_CONFIG_FILE%"
    call :Log "Modify upgrade status finish."


    md "%AGENT_PUSHUPGRADE_LOG_PATH%"
    call :Log "Backup push upgrade log, and push upgrade process ends."
    if exist "%CURRENT_PATH%\upgrade.log" (
        copy /y "%CURRENT_PATH%\upgrade.log" "%AGENT_PUSHUPGRADE_LOG_PATH%\upgrade.log"  >> %LOGFILE_PATH% 2>&1
    )
    if exist "%CURRENT_PATH%\upgrade.log" (
        copy /y "%CURRENT_PATH%\agent_upgrade.log" "%AGENT_PUSHUPGRADE_LOG_PATH%\agent_upgrade.log"  >> %LOGFILE_PATH% 2>&1
    )
    if exist "%CURRENT_PATH%\agent_upgrade_sqlite.log" (
        copy /y "%CURRENT_PATH%\agent_upgrade_sqlite.log" "%AGENT_PUSHUPGRADE_LOG_PATH%\agent_upgrade_sqlite.log"  >> %LOGFILE_PATH% 2>&1
    )
    call :CopyLogFile

    start /min cmd.exe /Q /C "timeout /T 3 /NOBREAK & if exist "%AGENT_PUSHUPGRADE_PACKAGE_PATH%" (rd /s /q "%AGENT_PUSHUPGRADE_PACKAGE_PATH%"  >> %LOGFILE_PATH% 2>&1) & exit 0"
    exit /b 0
goto :EOF

:LogError
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_update_prepare_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
goto :EOF

:CheckAndStartAgent
    if %AGENT_IS_STOP% EQU 1 (
        rem start Agent
        call :Log "Exec agent_start.bat, in RollBack."
        call "%IN_USE_AGENT_ROOT_PATH%\bin\agent_start.bat" upgrade
        if NOT %errorlevel% EQU 0 (
            call :Log "Exec agent_start.bat failed"
            exit /b 1
        )
        call :Log "Exec agent_start.bat successfully.""
    )
goto :EOF

:ExecExit
    call :Log "Upgrade.bat exec ExecExit"
    if %IS_CLEAN% EQU 1 (
        call :ClearSource
    )

    rem push upgrade
    if %UPGRADDE_TYPE% EQU 1 (
        call :UpdateUpgradeStatus
        if %IS_SUCC% EQU 1 (
            cd "%AGENT_ROOT_PATH%"
        )
        call :CheckAndStartAgent
        endlocal
        exit /b %1
    )

    rem manual upgrade
    if %IS_SUCC% EQU 1 (
        echo ####################Upgrade succeed##########################
        call :Log "####################Upgrade succeed##########################"
    ) else (
        call :CheckAndStartAgent
        echo ####################Upgrade failed##########################
        call :Log "####################Upgrade failed##########################"
    )
    call :CopyLogFile
    call :Log "Exit upgrade.bat"
    echo "The process is complete, press any key to exit"
    pause
    endlocal
exit /b 0