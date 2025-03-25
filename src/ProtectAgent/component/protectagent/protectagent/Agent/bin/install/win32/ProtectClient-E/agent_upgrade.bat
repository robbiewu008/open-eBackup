@echo off
rem ########################################################################
rem #
rem #  %~1:installtion package path
rem #  %~2:upgrade type
rem #
rem #  usage:
rem #         common upgrade : agent_upgrade.bat path 0
rem #         push upgrade : agent_upgrade.bat path 1
rem #
rem ########################################################################

setlocal EnableDelayedExpansion

rem ------------------ new pkg path -------------------------
set CURRENT_PATH=%~dp0
rem script can use variable of father script(upgrade.bat)
set AGENT_NEWPKG_BIN_PATH=%CURRENT_PATH%
set AGENT_NEWPKG_TMP_PATH=%AGENT_NEWPKG_ROOT_PATH%\tmp
set AGENT_NEWPKG_INSTALL_CONF_PATH=%AGENT_NEWPKG_PLACE_PATH%\conf

set LOGFILE_PATH=%AGENT_NEWPKG_PLACE_PATH%\agent_upgrade.log
set COPYLOG_PATH=%AGENT_NEWPKG_PLACE_PATH%\agent_robocopy.log

rem ------------------- in use pkg ------------------------------
set XML_CFG_EXE=%IN_USE_AGENT_ROOT_PATH%\bin\xmlcfg.exe

rem ----------------------- backup path, use target install path ----------
set AGENT_BACKUP_PATH=%AGENT_PKG_INSTALL_PATH%\AgentUpgrade
set OLD_AGENT_BACKUP_PATH=%AGENT_BACKUP_PATH%\Old
set NEW_AGENT_BACKUP_PATH=%AGENT_BACKUP_PATH%\New
set AGENT_COPY_MERGE_PATH=%AGENT_BACKUP_PATH%\merge

for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set "Current_Time=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"
set BACKUP_COPY_PATH=%AGENT_PKG_INSTALL_PATH%\Bak\%Current_Time%

set UPGRADDE_TYPE=%~2
set AGENT_ROLE=
set PM_IP=
set USER_ID=
set IS_SUCC=0
set LOOP_COUNT=0
set IS_IMPORTCRL=0
set IS_CLEAN=0

if exist "%LOGFILE_PATH%" (
    del "%LOGFILE_PATH%"
)

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

set /a ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER=1577209881
set /a ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM=1577209878
set /a ERR_UNZIP_PKG=1677931367
set /a ERR_UPGRADE_PKG_VERSION_LOW=1577210139
set /a ERR_EXEC_REGISTER=1577210142
set /a ERR_UPGRADE_PKG_VERSION_LOW_RETCODE=79
rem -------------error code----------------------

rem #################### Main process ##########################
echo ####################Agent_upgrade begin##########################
call :Log "####################Agent_upgrade begin##########################"

call :Log "Exec CheckParam function."
call :CheckParam
if NOT %errorlevel% EQU 0 (
    call :Log "Exec CheckParam function failed"
    call :ExecExit
    exit /b 1
)
call :Log "Exec CheckParam function succ."

echo "    step7.1: Backup current DataBackup ProtectAgent"
call :Log "Exec BackupInstalled function."
call :BackupInstalled
if NOT %errorlevel% EQU 0 (
    echo "    step7.1: failed"
    call :Log "Exec BackupInstalled function failed."
    call :ExecExit
    exit /b 1
)
call :Log "Exec BackupInstalled function succ."
echo "    step7.1: backup agent data succesfully."

echo "    step7.2: Upgrade Handle is in progress"
call :Log "Exec SetUpgradeConfig function."
call :SetUpgradeConfig
if NOT %errorlevel% EQU 0 (
    echo "    step7.2: failed"
    call :Log "Exec SetUpgradeConfig function failed"
    call :ExecExit
    exit /b 1
)
call :Log "Exec SetUpgradeConfig function succ."
echo "    step7.2: configure agent upgrade successfully."

call :Log "Exec uninstall.bat"
call "%IN_USE_AGENT_MANAGER_PATH%\uninstall.bat" upgrade
if NOT %errorlevel% EQU 0 (
    echo "    step7.3: Exec uninstall.bat failed, and begin exec RollBack function"
    call :Log "Exec uninstall.bat failed, and begin exec RollBack function"
    call :RollBack
    if NOT !errorlevel! EQU 0 (
        echo "    step7.3: Exec uninstall.bat failed, and exec RollBack function failed"
        call :Log "Exec uninstall.bat failed, and exec RollBack function failed"
        call :ExecExit
        exit /b 1
    ) else (
        call :Log "Exec uninstall.bat failed, and exec RollBack function succeed"
        call :ExecExit
        exit /b 2
    )
)
call :Log "Exec uninstall.bat succ."
echo "    step7.3: uninstall agent successfully."

call :Log "Exec install.bat"
call "%AGENT_NEWPKG_PLACE_PATH%\install.bat" upgrade
if %errorlevel% EQU 0 (
    call :CheckUpgradeResult
)
if NOT %errorlevel% EQU 0 (
    echo "    step7.4: Exec install.bat failed, and exec RollBack function"
    call :Log "Exec install.bat failed, and exec RollBack function"
    call :RollBack
    if NOT !errorlevel! EQU 0 (
        echo "    step7.4: Exec install.bat failed, and exec RollBack function failed"
        call :Log "Exec install.bat failed, and exec RollBack function failed"
        call :ExecExit
        exit /b 1
    ) else (
        call :Log "Exec install.bat failed, and exec RollBack function succeed"
        call :ExecExit
        exit /b 2
    )
)
call :Log "Exec install.bat succ."
echo "    step7.4: install agent successfully."

call :Log "Exec UpgradeAfterWork function"
call :UpgradeAfterWork
if NOT %errorlevel% EQU 0 (
    echo "    step7.5: Exec UpgradeAfterWork function failed, and exec RollBack function"
    call :Log "Exec UpgradeAfterWork function failed, and exec RollBack function"
    call :RollBack
    if NOT !errorlevel! EQU 0 (
        echo "    step7.5: Exec UpgradeAfterWork function failed, and exec RollBack function failed"
        call :Log "Exec UpgradeAfterWork function failed, and exec RollBack function failed"
        call :ExecExit
        exit /b 1
    ) else (
        call :Log "Exec UpgradeAfterWork function failed, and exec RollBack function succeed"
        call :ExecExit
        exit /b 2
    )
)
echo "    step7.5: configure agent successfully."
call :Log "step7.5: configure agent successfully."

set IS_SUCC=1
call :Log "Exec ExecExit function"
rem when upgrade succeed, clean tmp files
set IS_CLEAN=1
call :ExecExit
exit /b 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
goto :EOF

:CheckParam
    if not "%UPGRADDE_TYPE%" EQU "" (
        if not "%UPGRADDE_TYPE%" EQU "0" (
            if not "%UPGRADDE_TYPE%" EQU "1" (
                call :Log "Parameter detection failed, please confirm the parameters."
                echo Parameter detection failed, please confirm the parameters.
                call :PrintUsage
                exit /b 1
            )
        )
    )
    exit /b 0
goto :EOF

:BackupInstalled
    rem backup installed Agent
    if exist "%OLD_AGENT_BACKUP_PATH%" (
        rd /s /q  "%OLD_AGENT_BACKUP_PATH%" >> %LOGFILE_PATH% 2>&1
    )
    robocopy "%IN_USE_AGENT_MANAGER_PATH%" "%OLD_AGENT_BACKUP_PATH%"  /E /PURGE /NP /LOG:%COPYLOG_PATH% /NDL /NFL /Z /R:5 /W:5
    if NOT %errorlevel% LEQ 7 (
        call :Log "Backup installed Agent failed."
        exit /b 1
    )
    call :Log "Backup installed Agent successfully."

    rem backup plugin file
    call :BackupPluginFiles

    call :Log "Backup plugin files successfully."

    rem backup client.conf of new package
    if exist "%NEW_AGENT_BACKUP_PATH%" (
        rd /s /q  "%NEW_AGENT_BACKUP_PATH%" >> %LOGFILE_PATH% 2>&1
    )
    md "%NEW_AGENT_BACKUP_PATH%"
    md "%NEW_AGENT_BACKUP_PATH%\conf\"
    copy /y "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.conf" "%NEW_AGENT_BACKUP_PATH%\conf\" >> %LOGFILE_PATH% 2>&1 && (
        call :Log "Backup upgrade package client.conf succ"
    )

    rem backup cert file in new pkg
    copy /y "%AGENT_NEWPKG_INSTALL_CONF_PATH%\ca.crt.pem" "%NEW_AGENT_BACKUP_PATH%\conf\" >> %LOGFILE_PATH% 2>&1
    copy /y "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.crt.pem" "%NEW_AGENT_BACKUP_PATH%\conf\" >> %LOGFILE_PATH% 2>&1
    copy /y "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.pem" "%NEW_AGENT_BACKUP_PATH%\conf\" >> %LOGFILE_PATH% 2>&1
    copy /y "%AGENT_NEWPKG_INSTALL_CONF_PATH%\agentca.crt.pem" "%NEW_AGENT_BACKUP_PATH%\conf\" >> %LOGFILE_PATH% 2>&1

    call :Log "Call GetSNMPCfg function"
    rem backup SNMP config
    call :GetSNMPCfg

    exit /b 0
goto :EOF

:SetUpgradeConfig
    if not exist "%IN_USE_AGENT_ROOT_PATH%\conf\agent_cfg.xml" (
        call :Log "Can't find agent_cfg.xml in Installtioned Agent"
        exit /b 1
    )
    if not exist "%XML_CFG_EXE%" (
        call :Log "Can't find xmlcfg.exe in Installtioned Agent"
        exit /b 1
    )
    if not exist "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.conf" (
        call :Log "Can't find client.conf in current path %AGENT_NEWPKG_INSTALL_CONF_PATH%."
        exit /b 1
    )

    rem check Agent role
    for /f %%a in ('call "%XML_CFG_EXE%" read Backup "backup_role"') do (set AGENT_ROLE=%%a)
    for /f "delims== tokens=1" %%a in (' findstr "backup_role" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.conf" ') do (
        if "%AGENT_ROLE%" EQU "%%a" (
            call :Log "The installation package does not match the installed agent role"
            exit /b 1
        )
    )

    rem get PM_IP and USER_ID from agent_cfg.xml
    for /f "usebackq tokens=1,2 delims== " %%a in ("!IN_USE_AGENT_ROOT_PATH!\conf\testcfg.tmp") do (
        if "%%a" == "PM_IP" (
            set PM_IP=%%b
        )
        if "%%a" == "PM_PORT" (
            set PM_PORT=%%b
        )
        if "%%a"=="PM_MANAGER_IP" (
            set PM_MANAGER_IP=%%b
        )
         if "%%a"=="IS_ENABLE_DATATURBO" (
            set IS_ENABLE_DATATURBO=%%b
        )
        if "%%a"=="PM_MANAGER_PORT" (
            set PM_MANAGER_PORT=%%b
        )
        if "%%a"=="EIP" (
            set EIP=%%b
        )
    )
    for /f %%a in ('call "%XML_CFG_EXE%" read System "userid"') do (set USER_ID=%%a)

    rem replace PM_IP USER_ID, replace client.conf
    if exist "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp" (
        del "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
    )

    for /f "delims== tokens=1,2" %%a in ('type "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.conf"') do ( 
        if "%%a" EQU "pm_ip" (
            echo %%a=%PM_IP%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else if "%%a" EQU "pm_manager_ip" ( 
            echo %%a=%PM_MANAGER_IP%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else if "%%a" EQU "pm_port" ( 
            echo %%a=%PM_PORT%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else if "%%a" EQU "pm_manager_port" (
            echo %%a=%PM_MANAGER_PORT%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else if "%%a" EQU "IS_ENABLE_DATATURBO" (
            echo %%a=%IS_ENABLE_DATATURBO%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else if "%%a" EQU "user_id" ( 
            echo %%a=%USER_ID%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else if "%%a" EQU "eip" ( 
            echo %%a=%EIP%>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        ) else (
            echo %%a=%%b>> "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp"
        )
    )
    move /y "%AGENT_NEWPKG_TMP_PATH%\cfg.tmp" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.conf" >> %LOGFILE_PATH% 2>&1

    rem replace certification, use the certification of old agent
    if exist "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\pmca.pem" (
        copy /y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\pmca.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\ca.crt.pem" >> %LOGFILE_PATH% 2>&1
        copy /y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\agentca.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\agentca.crt.pem" >> %LOGFILE_PATH% 2>&1
    ) else (
        rem Compatible certificate system upgrade
        copy /y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\bcmagentca.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\ca.crt.pem" >> %LOGFILE_PATH% 2>&1
        copy /y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\bcmagentca.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\agentca.crt.pem" >> %LOGFILE_PATH% 2>&1
    )
    copy /y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\server.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.crt.pem" >> %LOGFILE_PATH% 2>&1
    copy /y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\server.key" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\client.pem" >> %LOGFILE_PATH% 2>&1

    exit /b 0
goto :EOF

:UpgradeAfterWork
    rem restore SNMP config
    call :SetSNMPCfg
    if NOT %errorlevel% EQU 0 (
        exit /b 1
    )
    rem upgrade SQLite
    call :RestoreSQLite
    if NOT %errorlevel% EQU 0 (
        exit /b 1
    )
    rem Restore CRL status
    call :RestoreCRL
    if NOT %errorlevel% EQU 0 (
        exit /b 1
    )

    call :Log "Exec UpgradeAfterWork function succ."
    exit /b 0
goto :EOF

:CheckAndKillProcess
    set LOOP_COUNT=0
    :loopcheckprocess
    if %LOOP_COUNT% GEQ 10 (
        call :Log "Closed process[%~1] timeout."
        exit /b 1
    )
    for /f "tokens=1-6" %%a in (' tasklist ^| findstr "%~1" ') do (
        if not "%%a" EQU "" (
            if not "%%b" EQU "" (
                taskkill /f /t /pid %%b  >> %LOGFILE_PATH% 2>&1
                timeout 1 /NOBREAK  >> %LOGFILE_PATH% 2>&1
                set /a LOOP_COUNT+=1
                goto :loopcheckprocess
            ) else (
                call :Log "process %~1 isn't running."
            )
        )
    )
    call :Log "Process[%~1] is closed."
    exit /b 0
goto :EOF

:CheckProcessStatus
    call :CheckAndKillProcess "monitor.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check monitor.exe status failed."
        exit /b 1
    )
    call :CheckAndKillProcess "rdagent.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check rdagent.exe status failed."
        exit /b 1
    )
    call :CheckAndKillProcess "rdnginx.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check rdnginx.exe status failed."
        exit /b 1
    )
goto :EOF

:RollBack
    rem backup failed log in upgrade 
    call :Log "Exec RollBack funciton, in RollBack."
    if exist "%IN_USE_AGENT_ROOT_PATH%\log\agent_uninstall.log" (
        copy /y "%IN_USE_AGENT_ROOT_PATH%\log\agent_uninstall.log" %AGENT_NEWPKG_PLACE_PATH%\agent_uninstall_in_upgrade.log  >> %LOGFILE_PATH% 2>&1
    )
    if exist "%AGENT_ROOT_PATH%\log\agent_install.log" (
        copy /y "%AGENT_ROOT_PATH%\log\agent_install.log" %AGENT_NEWPKG_PLACE_PATH%\agent_install_in_upgrade.log  >> %LOGFILE_PATH% 2>&1
    )

    rem stop Agent
    call :Log "Exec agent_stop.bat, in RollBack."
    call "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\bin\agent_stop.bat" upgrade
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec agent_stop.bat failed."
        exit /b 1
    )

    rem Confirm that the process is closed
    echo "Confirm that the process is closed, please wait."
    call :Log "Exec CheckProcessStatus funciton, in RollBack."
    call :CheckProcessStatus
    if NOT %errorlevel% EQU 0 (
        echo "Confirm the process status failed."
        call :Log "Exec CheckProcessStatus failed."
        exit /b 1
    )

    rem restore Agent
    call :Log "Restore installed Agent, in RollBack."
    if exist "%AGENT_MANAGER_PATH%" (
        rd /s /q  "%AGENT_MANAGER_PATH%"  >> %LOGFILE_PATH% 2>&1
    )
    if exist "%IN_USE_AGENT_MANAGER_PATH%" (
        rd /s /q  "%IN_USE_AGENT_MANAGER_PATH%"  >> %LOGFILE_PATH% 2>&1
    )
    md "%IN_USE_AGENT_MANAGER_PATH%"
    xcopy /e/h/k/x/o/q/y "%OLD_AGENT_BACKUP_PATH%\*" "%IN_USE_AGENT_MANAGER_PATH%"  >> %LOGFILE_PATH% 2>&1
    echo Y | Cacls "%IN_USE_AGENT_MANAGER_PATH%" /T /E /R Users  >> %LOGFILE_PATH% 2>&1

    rem start Agent
    call :Log "Exec agent_start.bat, in RollBack."
    call "%IN_USE_AGENT_ROOT_PATH%\bin\agent_start.bat" upgrade
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec agent_start.bat failed"
        exit /b 1
    )
    set AGENT_IS_STOP=0

    rem Register Agent to PM 
    call :Log "Exec RegisterAgent funciton, in RollBack."
    call "%IN_USE_AGENT_ROOT_PATH%\bin\agentcli.exe" registerHost RegisterHost
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec RegisterAgent failed"
        exit /b 1
    )
    exit /b 0
goto :EOF

:GetSNMPCfg
    if exist "%AGENT_NEWPKG_TMP_PATH%\System.conf" (
        del "%AGENT_NEWPKG_TMP_PATH%\System.conf"
    )
    if exist "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf" (
        del "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf"
    )
    if exist "%AGENT_NEWPKG_TMP_PATH%\Backup.conf" (
        del "%AGENT_NEWPKG_TMP_PATH%\Backup.conf"
    )
    if exist "%AGENT_NEWPKG_TMP_PATH%\Mount.conf" (
        del "%AGENT_NEWPKG_TMP_PATH%\Mount.conf"
    )
    for /f %%a in ('call "%XML_CFG_EXE%" read System "log_level"') do (echo log_level=%%a>> "%AGENT_NEWPKG_TMP_PATH%\System.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read System "log_count"') do (echo log_count=%%a>> "%AGENT_NEWPKG_TMP_PATH%\System.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read System "log_max_size"') do (echo log_max_size=%%a>> "%AGENT_NEWPKG_TMP_PATH%\System.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read System "log_keep_time"') do (echo log_keep_time=%%a>> "%AGENT_NEWPKG_TMP_PATH%\System.conf")

    for /f %%a in ('call "%XML_CFG_EXE%" read Monitor rdagent "logfile_size"') do (echo logfile_size=%%a>> "%AGENT_NEWPKG_TMP_PATH%\Monitor.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read Monitor rdagent "logfile_size_alarm_threshold"') do (echo logfile_size_alarm_threshold=%%a>> "%AGENT_NEWPKG_TMP_PATH%\Monitor.conf")
    
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "engine_id"') do (echo engine_id=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "context_name"') do (echo context_name=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "security_name"') do (echo security_name=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "security_model"') do (echo security_model=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "security_level"') do (echo security_level=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "private_password"') do (echo private_password=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "private_protocol"') do (echo private_protocol=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "auth_password"') do (echo auth_password=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read SNMP "auth_protocol"') do (echo auth_protocol=%%a>> "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf")

    for /f %%a in ('call "%XML_CFG_EXE%" read Backup "archive_threshold"') do (echo archive_threshold=%%a>> "%AGENT_NEWPKG_TMP_PATH%\Backup.conf")
    for /f %%a in ('call "%XML_CFG_EXE%" read Backup "archive_thread_timeout"') do (echo archive_thread_timeout=%%a>> "%AGENT_NEWPKG_TMP_PATH%\Backup.conf")

    for /f %%a in ('call "%XML_CFG_EXE%" read Mount "win_mount_public_path"') do (echo win_mount_public_path=%%a>> "%AGENT_NEWPKG_TMP_PATH%\Mount.conf")
    exit /b 0
goto :EOF

:SetSNMPCfg
    if not exist "%AGENT_NEWPKG_TMP_PATH%\System.conf" (
        call :Log "Can't find System.conf in temporary decompression directory"
        exit /b 1
    )
    if not exist "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf" (
        call :Log "Can't find SNMP.conf in temporary decompression directory"
        exit /b 1
    )
    if not exist "%AGENT_NEWPKG_TMP_PATH%\Backup.conf" (
        call :Log "Can't find Backup.conf in temporary decompression directory"
        exit /b 1
    )
    for /f "delims== tokens=1,2" %%a in ('type "%AGENT_NEWPKG_TMP_PATH%\System.conf"') do ( 
        call "%AGENT_ROOT_PATH%\bin\xmlcfg.exe" write System %%a "%%b"
    )
    for /f "delims== tokens=1,2" %%a in ('type "%AGENT_NEWPKG_TMP_PATH%\Monitor.conf"') do ( 
        call "%AGENT_ROOT_PATH%\bin\xmlcfg.exe" write Monitor rdagent %%a "%%b"
    )
    for /f "delims== tokens=1,2" %%a in ('type "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf"') do ( 
        call "%AGENT_ROOT_PATH%\bin\xmlcfg.exe" write SNMP %%a "%%b"
    )
    for /f "delims== tokens=1,2" %%a in ('type "%AGENT_NEWPKG_TMP_PATH%\Backup.conf"') do ( 
        call "%AGENT_ROOT_PATH%\bin\xmlcfg.exe" write Backup %%a "%%b"
    )
    for /f "delims== tokens=1,2" %%a in ('type "%AGENT_NEWPKG_TMP_PATH%\Mount.conf"') do (
        if not "%%b" == "Section" (
            call "%AGENT_ROOT_PATH%\bin\xmlcfg.exe" write Mount %%a %%b
        )
    )
    del "%AGENT_NEWPKG_TMP_PATH%\SNMP.conf"
    del "%AGENT_NEWPKG_TMP_PATH%\Monitor.conf"
    del "%AGENT_NEWPKG_TMP_PATH%\Backup.conf" 
    del "%AGENT_NEWPKG_TMP_PATH%\Mount.conf" 
    exit /b 0
goto :EOF

:PrintUsage
    echo  Usage:
    echo      common upgrade : agent_upgrade.bat path 0
    echo      push upgrade : agent_upgrade.bat path 1
    exit /b 0
goto :EOF

rem restore 1 to 2, then delete
:RestoreOneFile
    if exist "%~1" (
        copy /y "%~1" "%~2" > nul
        del "%~1"
    )
    exit /b 0
goto :EOF

:RestoreEnv
    rem restore file in new pkg
    call :RestoreOneFile "%NEW_AGENT_BACKUP_PATH%\conf\ca.crt.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\"
    call :RestoreOneFile "%NEW_AGENT_BACKUP_PATH%\conf\client.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\"
    call :RestoreOneFile "%NEW_AGENT_BACKUP_PATH%\conf\client.crt.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\"
    call :RestoreOneFile "%NEW_AGENT_BACKUP_PATH%\conf\agentca.crt.pem" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\"
    if exist "%NEW_AGENT_BACKUP_PATH%\conf\client.conf" (
        copy /y "%NEW_AGENT_BACKUP_PATH%\conf\client.conf" "%AGENT_NEWPKG_INSTALL_CONF_PATH%\"  >> %LOGFILE_PATH% 2>&1
    )

    call :RestorePluginFiles

    exit /b 0
goto :EOF

:RestoreSQLite
    xcopy /e/h/k/x/o/q/y "%AGENT_NEWPKG_ROOT_PATH%\db\*" "%NEW_AGENT_BACKUP_PATH%\db\" >> %LOGFILE_PATH% 2>&1

    xcopy /e/h/k/x/o/q/y "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\db\*" "%AGENT_NEWPKG_ROOT_PATH%\db\"  >> %LOGFILE_PATH% 2>&1
    call "%AGENT_NEWPKG_BIN_PATH%\agent_upgrade_sqlite.bat" "%AGENT_NEWPKG_ROOT_PATH%\db\" "%AGENT_ROOT_PATH%\db\"
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec agent_upgrade_sqlite.bat failed"
        exit /b 1
    )

    xcopy /e/h/k/x/o/q/y "%NEW_AGENT_BACKUP_PATH%\db\*" "%AGENT_NEWPKG_ROOT_PATH%\db\" >> %LOGFILE_PATH% 2>&1

    copy /y "%AGENT_NEWPKG_ROOT_PATH%\log\agent_upgrade_sqlite.log" "%AGENT_NEWPKG_PLACE_PATH%" >> %LOGFILE_PATH% 2>&1
    exit /b 0
goto :EOF

:RestoreCRL
    for /f "tokens=1,2 delims== " %%a in ('type "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\conf\testcfg.tmp" ^| findstr /c:"CERTIFICATE_REVOCATION="') do (
        if NOT "%%b" EQU "1" (
            call :Log "CRL status is [%%b], not need to reimport CRL."
            exit /b 0
        )
    )
    call "%AGENT_MANAGER_PATH%\crl_update.bat" -u "%OLD_AGENT_BACKUP_PATH%\ProtectClient-E\nginx\conf\server.crl"
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec crl_update.bat failed."
        exit /b 1
    ) 
    call :Log "Exec crl_update.bat successfully."
    exit /b 0
goto :EOF

rem copy or merge file from old dir to new dir
:MergeFilesInDir
    set oldDirPath=%~1
    set newDirPath=%~2
    call :Log "Start to merge srcDir %oldDirPath% to dstDir %newDirPath%."
    for /f %%i in ('dir /b/a:-d "%oldDirPath%" ') do (
        set fileName=%%i
        set newFilePath=!newDirPath!\!fileName!
        set oldFilePath=!oldDirPath!\!fileName!
        if exist "!newFilePath!" (
            rem merge log file
            set tmpFilePath=!AGENT_COPY_MERGE_PATH!\!fileName!
            copy /y  !oldFilePath!  !tmpFilePath!  >> %LOGFILE_PATH% 2>&1
            type !newFilePath! >> !tmpFilePath!
            copy /y  !tmpFilePath!  !newFilePath!  >> %LOGFILE_PATH% 2>&1
        ) else (
            rem copy log file
            copy /y !oldFilePath!  !newFilePath!  >> %LOGFILE_PATH% 2>&1
        )
    )
    call :Log "Merge srcDir %oldDirPath% to dstDir %newDirPath% finish."
goto :EOF

:MergeAgentLog
    call :Log "Start to merge log."

    md %AGENT_COPY_MERGE_PATH%

    set oldRootPath=%OLD_AGENT_BACKUP_PATH%\ProtectClient-E
    set newRootPath=%AGENT_ROOT_PATH%

    rem merge agent log
    call :MergeFilesInDir  %oldRootPath%\log  %newRootPath%\log

    rem merge plugin log
    for /f %%i in ('dir /b/a:d %oldRootPath%\log\Plugins') do (
        call :MergeFilesInDir  %oldRootPath%\log\Plugins\%%i  %newRootPath%\log\Plugins\%%i
    )

    rd /s /q %AGENT_COPY_MERGE_PATH%
    call :Log "Merge log finish."
    exit /b 0
goto :EOF

:BackupPluginFiles
    if exist "%OLD_AGENT_BACKUP_PATH%\Plugins\GeneralDBPlugin\bin\applications\common\sqlite" (
        md "%BACKUP_COPY_PATH%\Plugins\GeneralDBPlugin\sqlite"
        xcopy /e/h/k/x/o/q/y  "%OLD_AGENT_BACKUP_PATH%\Plugins\GeneralDBPlugin\bin\applications\common\sqlite" "%BACKUP_COPY_PATH%\Plugins\GeneralDBPlugin\sqlite"  >> %LOGFILE_PATH% 2>&1
        if NOT %errorlevel% EQU 0 (
            call :Log "Backup plugin files failed."
            exit /b 1
        )
    )
goto :EOF

:RestorePluginFiles
    if exist "%BACKUP_COPY_PATH%\Plugins\GeneralDBPlugin\sqlite" (
        if exist "%PLUGIN_DIR%\GeneralDBPlugin\bin\applications\common\sqlite" (
            xcopy /e/h/k/x/o/q/y  "%BACKUP_COPY_PATH%\Plugins\GeneralDBPlugin\sqlite\*" "%PLUGIN_DIR%\GeneralDBPlugin\bin\applications\common\sqlite"  >> %LOGFILE_PATH% 2>&1
        )
    )
goto :EOF

:CheckProcess
    tasklist | findstr "%~1"
    if NOT %errorlevel% EQU 0 (
        exit /b 1
    )
    exit /b 0

:CheckUpgradeResult
    call:Log "Start check upgrade status."

    call :CheckProcess "monitor.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check monitor.exe status failed."
        exit /b 1
    )

    call :CheckProcess "rdagent.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check rdagent.exe status failed."
        exit /b 1
    )

    call :CheckProcess "rdnginx.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check rdnginx.exe status failed."
        exit /b 1
    )

    call:Log "Check upgrade status successfully."
    exit /b 0

:ExecExit
    call :Log "agent_upgrade.bat exec ExecExit"
    call :RestoreEnv
    call :MergeAgentLog
    if %IS_CLEAN% EQU 1 (
        rd /s /q %AGENT_BACKUP_PATH%
    )
    if %IS_SUCC% EQU 1 (
        call :Log "Upgrade agent_upgrade successfully"
        echo ####################Agent_upgrade succ##########################
        call :Log "####################Agent_upgrade succ##########################" 
    ) else (
        call :Log "Upgrade agent_upgrade failed"
        echo ####################Agent_upgrade failed##########################
        call :Log "####################Agent_upgrade failed##########################" 
    )
    endlocal
goto :EOF