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
rem -----------------------------------------------------------
rem the small pkg uninstall(AI_SHU)
rem -----------------------------------------------------------

setlocal EnableDelayedExpansion

set CURRENT_PATH=%~dp0

rem ---------------- path -------------------------------------
set AGENT_MANAGER_PATH=%CURRENT_PATH%
set AGENT_ROOT_PATH=%AGENT_MANAGER_PATH%\ProtectClient-E\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\log\
set AFENT_TMP_PATH=%AGENT_ROOT_PATH%\tmp\
set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set AGENT_NGINX_CONF_PATH=%AGENT_ROOT_PATH%\nginx\conf\

set LOG_FILE=%AGENT_LOG_PATH%\updateCert.log

rem ---------------- bat and exe file ------------------------
set AGENTCLI_EXE=%AGENT_BIN_PATH%\agentcli.exe
set XMLCFG_EXE=%AGENT_BIN_PATH%\xmlcfg.exe

set AGENT_START_SCRIPT=%AGENT_BIN_PATH%\agent_start.bat
set AGENT_STOP_SCRIPT=%AGENT_BIN_PATH%\agent_stop.bat

rem ----------------- cert -----------------------------------
set CERT_BACKUP_PATH=%AFENT_TMP_PATH%\cert_backup\
set PM_CA_FILE=%AGENT_NGINX_CONF_PATH%\pmca.pem
set PM_CA_FILE_BAK=%CERT_BACKUP_PATH%\pmca.pem.bak
set AGENT_CA_FILE=%AGENT_NGINX_CONF_PATH%\agentca.pem
set AGENT_CA_FILE_BAK=%CERT_BACKUP_PATH%\agentca.pem.bak
set CERT_FILE=%AGENT_NGINX_CONF_PATH%\server.pem
set CERT_FILE_BAK=%CERT_BACKUP_PATH%\server.pem.bak
set KEY_FILE=%AGENT_NGINX_CONF_PATH%\server.key
set KEY_FILE_BAK=%CERT_BACKUP_PATH%\server.key.bak


@rem falg: 0 is false, 1 is ture
set NEED_CLEAN=0
set IS_SUCC=0
set PRE_PKEY_PASSWORD=
set PKEY_PASSWORD=

@rem cert update type
set UPDATE_TYPE=
@rem 1 : update Agent cert chain (file: agentca.pem, server.pem, server.key)
set UPDATE_AGENT_CHAIN=1
@rem 2 : update PM CAfile (file: pmca.pem)
set UPDATE_PMCA=2

rem #################### Main process ##########################
echo ####################updateCert begin##########################

net.exe session 1>NUL 2>NUL || (
    echo Please run the script as an administrator.
    pause
    exit /b 1
)

rem 1.make sure is update pems
call :IsUpdateCerts
if NOT %errorlevel% EQU 0 (
    goto :ExecExit
    exit /b 1
)

call :CheckInstalled
if NOT %errorlevel% EQU 0 (
    goto :ExecExit
    exit /b 1
)

rem 2.backup pems
call :BackupCerts
set NEED_CLEAN=1

rem 3. get pem path and copt pems into install path
call :GetCertPathAndOverride
if NOT %errorlevel% EQU 0 (
    goto :ExecExit
    exit /b 1
)

rem 4.update Huawei pems
call :UpdateCert
if NOT %errorlevel% EQU 0 (
    call :RollBackCerts
    if !errorlevel! EQU 0 (
        echo "Failed to update the certificates,and certificates rolled back successfully."
    ) else (
        echo "Certificate rolled back failed. Please manually roll back the certificate."
    )
    exit /b 1
)

rem 5.restart agent services
call :RestartAgent
if NOT %errorlevel% EQU 0 (
    goto :ExecExit
    exit /b 1
)

echo Succeed upgradeing the client certificate.
set IS_SUCC=1
call :ExecExit
exit /b 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOG_FILE%"
    goto :EOF

:IsUpdateCerts
    echo [Info]This script replaces only the client certificate and does not verify the certificate.
    echo [Info]Please ensure that the new certificate and the OpenApi component certificate of ProtectManager are issued by the same CA.
    echo Updating certificates would restart DataBackup ProtectAgent, are you sure you want to start updating the certificates now?[y/n]
    for %%a in (1 2 3) do (
        set /p choice="try %%a/3 >>"
        if "!choice!" EQU "y" (
            echo "Start update the certificates."
            call :Log "Start update the certificates."
            goto :imput_update_type
        )
        if "!choice!" EQU "n" (
            echo "Stop update the certificates."
            call :Log "Stop update the certificates."
            exit /b 1
        )
        echo "invalid input"
    )
    :imput_update_type
    echo "Choice update type[try ${i}/3]:\n"
    echo "    1: Update Agent CAfile and server certificate.\n"
    echo "    2: Only update PM CAfile.\n"
    echo "    n: Exit the program.\n"
    echo "input your choice?[1|2]:"
    for %%a in (1 2 3) do (
        set /p UPDATE_TYPE="try %%a/3 >>"
        if "!UPDATE_TYPE!" EQU "%UPDATE_AGENT_CHAIN%" (
            echo echo "user input is !UPDATE_TYPE!: Update Agent CAfile and server certificate."
            call :Log "user input is !UPDATE_TYPE!: Update Agent CAfile and server certificate."
            exit /b 0
        )
        if "!UPDATE_TYPE!" EQU "%UPDATE_PMCA%" (
            echo "user input is !UPDATE_TYPE!: Only update PM CAfile."
            call :Log "user input is !UPDATE_TYPE!: Only update PM CAfile."
            exit /b 0
        )
        echo "pleas input update type."
    )
    echo "Invalid input more than 3 times."
    exit /b 1
    
:CheckInstalled
    if NOT exist "%AGENT_ROOT_PATH%" (
        echo "DataBackup ProtectAgent is not detected in the current environment."
        call :Log "Check DataBackup ProtectAgent failed."
        exit /b 1
    )
    if NOT exist "%AGENTCLI_EXE%" (
        echo "Can not find agentcli.exe, and DataBackup ProtectAgent is incomplete."
        call :Log "Can not find agentcli."
        exit /b 1
    )
    if NOT exist "%XMLCFG_EXE%" (
        echo "Can not find xmlcfg.exe, and DataBackup ProtectAgent is incomplete."
        call :Log "Can not find xmlcfg."
        exit /b 1
    )
    exit /b 0

:BackupCerts
    if NOT exist %CERT_BACKUP_PATH% (
        mkdir %CERT_BACKUP_PATH%
        call :Log "Create certificates backup for the DataBackup ProtectAgent."
    )
    if "%UPDATE_TYPE%" EQU "%UPDATE_AGENT_CHAIN%" (
        copy /y "%AGENT_CA_FILE%" "%AGENT_CA_FILE_BAK%" >> "%LOG_FILE%" 2>&1
        copy /y "%CERT_FILE%" "%CERT_FILE_BAK%" >> "%LOG_FILE%" 2>&1
        copy /y "%KEY_FILE%" "%KEY_FILE_BAK%" >> "%LOG_FILE%" 2>&1
        call :Log "Backup certificate files successfully."
        exit /b 0
    )
    if "%UPDATE_TYPE%" EQU "%UPDATE_PMCA%" (
        copy /y "%PM_CA_FILE%" "%PM_CA_FILE_BAK%" >> "%LOG_FILE%" 2>&1
        call :Log "Backup certificate files successfully."
        exit /b 0
    )
    exit /b 1

:GetCertPathAndOverride
    echo "Please enter the path of the certificate to be upgraded:"
    echo "    update Agent cert chain: [client.pem, client.crt.pem, agentca.crt.pem]"
    echo "    update PM CAfile:        [ca.crt.pem]"
    set /p certPath=">>"
    if NOT exist "%certPath%" (
        echo "The input path does not exist."
        exit /b 1
    )

    if "%UPDATE_TYPE%" EQU "%UPDATE_AGENT_CHAIN%" (
        if NOT exist "%certPath%\client.pem" (
            echo "Can not find client.pem in the input path."
            exit /b 1
        )
        if NOT exist "%certPath%\client.crt.pem" (
            echo "Can not find client.crt.pem in the input path."
            exit /b 1
        )
        if NOT exist "%certPath%\agentca.crt.pem" (
            echo "Can not find agentca.crt.pem in the input path."
            exit /b 1
        )
        copy /y "%certPath%\agentca.crt.pem" "%AGENT_CA_FILE%" >> "%LOG_FILE%" 2>&1
        copy /y "%certPath%\client.crt.pem" "%CERT_FILE%" >> "%LOG_FILE%" 2>&1
        copy /y "%certPath%\client.pem" "%KEY_FILE%" >> "%LOG_FILE%" 2>&1
    )
    if "%UPDATE_TYPE%" EQU "%UPDATE_PMCA%" (
        if NOT exist "%certPath%\ca.crt.pem" (
            echo "Can not find ca.crt.pem in the input path."
            exit /b 1
        )
        copy /y "%certPath%\ca.crt.pem" "%PM_CA_FILE%" >> "%LOG_FILE%" 2>&1
    )
    echo "Update the certificate files successfully."
    exit /b 0

:VerifyPasswd
    "%AGENTCLI_EXE%" verifykey | findstr /c:"failed" > nul && (
        exit /b 1
    )
    exit /b 0

:GetSslKeyAndWriteXml
    echo "Please enter the private key password of the certificate:"
    "%AGENTCLI_EXE%" enckey cipherFile
    if NOT exist "%AFENT_TMP_PATH%\input_tmpcipherFile" (
        call :Log "Invoke agentcli.exe to enckey the private key password failed."
        exit /b 1
    )
    for /f %%a in ('type "%AFENT_TMP_PATH%\input_tmpcipherFile"') do set PKEY_PASSWORD=%%a
    del /f /q "%AFENT_TMP_PATH%\input_tmpcipherFile"
    if "!PKEY_PASSWORD!" EQU "" (
        call :Log "Failed to get encrypted content."
        exit /b 1
    )
    for /f %%a in ('call "%XMLCFG_EXE%" read Monitor nginx "ssl_key_password"') do set PRE_PKEY_PASSWORD=%%a
    "%XMLCFG_EXE%" write Monitor nginx ssl_key_password "%PKEY_PASSWORD%"
    set PKEY_PASSWORD=

    call :VerifyPasswd
    if NOT %errorlevel% EQU 0 (
        echo "Failed to verify the private key password."
        call :Log "Failed to verify the private key password."
        exit /b 1
    )
    set PRE_PKEY_PASSWORD=
    echo "Update the private key password of the certificate successfully."
    call :Log "Update the private key password of the certificate successfully."
    exit /b 0

:RegisterHost
    "%AGENTCLI_EXE%" registerHost RegisterHost >> "%LOG_FILE%" 2>&1
    if NOT %errorlevel% EQU 0 (
        echo "Invoke agentcli.exe to register Host failed."
        call :Log "Invoke agentcli.exe to register Host failed."
        exit /b 1
    )
    echo "Invoke agentcli.exe to register Host successfully."
    call :Log "Invoke agentcli.exe to register Host successfully."
    exit /b 0

:UpdateCert
    if "%UPDATE_TYPE%" EQU "%UPDATE_AGENT_CHAIN%" (
        call :GetSslKeyAndWriteXml
        if NOT !errorlevel! EQU 0 (
            echo "Failed to update the private key password."
            call :Log "Failed to update the private key password."
            exit /b 1
        )
    )

    call :RegisterHost
    if NOT %errorlevel% EQU 0 (
        echo "Register to ProtectManager failed."
        call :Log "Register to ProtectManager failed."
        exit /b 1
    )
    exit /b 0

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
                taskkill /f /pid %%b >> "%LOG_FILE%" 2>&1
                timeout 1 /NOBREAK >> "%LOG_FILE%" 2>&1
                set /a LOOP_COUNT+=1
                goto :loopcheckprocess
            ) else (
                call :Log "Process[%~1] is not running." 
            )
        )
    )
    call :Log "Process[%~1] is closed."
    exit /b 0

:CheckProcessStatus
    call :CheckAndKillProcess "monitor.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check monitor.exe status failed."
        exit /b 1
    )
    call :CheckAndKillProcess "rdagent.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check monitor.exe status failed."
        exit /b 1
    )
    call :CheckAndKillProcess "rdnginx.exe"
    if NOT %errorlevel% EQU 0 (
        call :Log "Check monitor.exe status failed."
        exit /b 1
    )
    echo "Check DataBackup ProtectAgent process completed."
    exit /b 0

:RestartAgent
    call %AGENT_STOP_SCRIPT%
    if NOT %errorlevel% EQU 0 (
        echo "Failed to stop the service."
        call :Log "Failed to stop the service."
        exit /b 1
    )
    call :CheckProcessStatus
    if NOT %errorlevel% EQU 0 (
        echo "Exec CheckProcessStatus failed."
        call :Log "Exec CheckProcessStatus failed."
        exit /b 1
    )
    call %AGENT_START_SCRIPT%
    if NOT %errorlevel% EQU 0 (
        echo "Failed to stop the service."
        call :Log "Failed to stop the service."
        exit /b 1
    )
    echo "Restart DataBackup ProtectAgent successfully."
    call :Log "Restart DataBackup ProtectAgent successfully."
    exit /b 0

:RollBackCerts
    echo "Start to rollback certificates."
    call :Log "Start to rollback certificates."
    if "%UPDATE_TYPE%" EQU "%UPDATE_AGENT_CHAIN%" (
        copy /y "%AGENT_CA_FILE_BAK%" "%AGENT_CA_FILE%" >> "%LOG_FILE%" 2>&1
        copy /y "%CERT_FILE_BAK%" "%CERT_FILE%" >> "%LOG_FILE%" 2>&1
        copy /y "%KEY_FILE_BAK%" "%KEY_FILE%" >> "%LOG_FILE%" 2>&1
        call :Log "Backup certificate files successfully."
    )
    if "%UPDATE_TYPE%" EQU "%UPDATE_PMCA%" (
        copy /y "%PM_CA_FILE_BAK%" "%PM_CA_FILE%" >> "%LOG_FILE%" 2>&1
        call :Log "Backup certificate files successfully."
    )
    "%XMLCFG_EXE%" write Monitor nginx ssl_key_password "%PRE_PKEY_PASSWORD%"
    set PRE_PKEY_PASSWORD=
    call :RestartAgent
    if NOT %errorlevel% EQU 0 (
        call :Log "Restart the DataBackup ProtectAgent failed."
        exit /b 1
    )
    echo "Rollback certificates successfully."
    call :Log "Certificate update failed,and rollback successfully."
    pause
    exit /b 0

:DeleteBackupCert
    if exist "%CERT_BACKUP_PATH%" (
        rd /s /q "%CERT_BACKUP_PATH%"
    )
    call :Log "Clean certificate backup files successfully."
    exit /b 0

:ExecExit
    if %NEED_CLEAN% EQU 1 (
        call :DeleteBackupCert
    )
    echo "The script is executed."
    call :Log "The script is executed."
    pause
    endlocal
    exit /b 0 
