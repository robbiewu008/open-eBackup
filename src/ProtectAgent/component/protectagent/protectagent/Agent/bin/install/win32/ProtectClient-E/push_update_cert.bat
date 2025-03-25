@echo off
setlocal EnableDelayedExpansion

set FALLBACK_TYPE=%~1
set JOB_ID=%~2
 
rem ------------------------- path -------------------------------------
set CURRENT_PATH=%~dp0
set AGENT_BIN_PATH=%CURRENT_PATH%
set AGENT_ROOT_PATH=%AGENT_BIN_PATH%\..
set AGENT_TPM_CART_UPDATE_DIR=%AGENT_ROOT_PATH%\tmp\cert_updating_%JOB_ID%\
set AGENT_TMP_NEW_CERT_PATH=%AGENT_TPM_CART_UPDATE_DIR%\newCert\
set AGENT_TMP_OLD_CERT_PATH=%AGENT_TPM_CART_UPDATE_DIR%\oldCert\
set STOP_SCRIPT=%AGENT_BIN_PATH%\agent_stop.bat
set START_SCRIPT=%AGENT_BIN_PATH%\agent_start.bat
set XMLCFG_EXE=%AGENT_BIN_PATH%\xmlcfg.exe
set AGENTCLI_EXE=%AGENT_BIN_PATH%\agentcli.exe
 
set AGENT_NGINX_PATH=%AGENT_ROOT_PATH%\nginx
set AGENT_NGINX_CONF_PATH=%AGENT_NGINX_PATH%\conf
 
set THRIFT_CERT_PATH=%AGENT_ROOT_PATH%\conf\thrift
 
set LOG_FILE_NAME=%AGENT_ROOT_PATH%\log\push_update_cert.log
 
rem -------------------------- para -------------------------------------
rem 1: update Agent cert chain (file: agentca.pem, server.pem, server.key)
set UPDATE_AGENT_CHAIN=1
rem 2: update PM CAfile (file: pmca.pem)
set UPDATE_PMCA=2
set UPDATE_TYPE=%UPDATE_AGENT_CHAIN%

rem update cert or rollback cert
set UPDATE_CERT=10
set ROLLBACK_CERT=11
set CLEAN_TMP_FILE=12
 
set PRE_PKEY_PASSWORD=

rem ------------------------- error code --------------------------------
set /a ERR_ROLL_BACK_SUCCESS=5
set /a ERR_ROLL_BACK_FAILED=6
 
rem ######################## Main process ###############################
echo ######################## updateCert begin ###############################
net.exe session 1>NUL 2>NUL || (
    echo Please run the script as an administrator.
    pause
    exit /b 1
)

rem clean temp files
if "%FALLBACK_TYPE%"=="%CLEAN_TMP_FILE%" (
    call :Log "************ Clean temp cert files ************"
    call :CleanTempFiles
    if NOT %errorlevel% EQU 0 (
        call :Log "Clean temp cert files failed!"
        exit /b 1
    )
    exit /b 0
)

rem update type
call :CheckUpdateType
if NOT %errorlevel% EQU 0 (
    call :Log "Cannot update cert files!"
    exit /b 1
)
 
rem check if rollback
if "%FALLBACK_TYPE%"=="%ROLLBACK_CERT%" (
    call :Log "************ Rollback cert ************"
    call :RollBackCert
    if NOT %errorlevel% EQU 0 (
        call :Log "Cannot update cert files!"
        exit /b %ERR_ROLL_BACK_FAILED%
    )
    call :Log "Rollback cert successfully!"
    exit /b %ERR_ROLL_BACK_SUCCESS%
)
 
call :Log "************ Update cert ************"
call :BackupOldCerts
if NOT %errorlevel% EQU 0 (
    call :Log "Backup old cert files failed!"
    exit /b 1
)
 
call :ReplaceWithNewCerts %AGENT_TMP_NEW_CERT_PATH%
if NOT %errorlevel% EQU 0 (
    call :Log "Replace old cert files failed!"
    call :RollBackCert
    exit /b 1
)

call :RestartAgent
if NOT %errorlevel% EQU 0 (
    call :Log "Restart agent failed!"
    call :RollBackCert
    exit /b 1
)
 
call :Log "Update certs successfully."
exit /b 0
rem ######################## Function ###############################
:Log
    echo "%~1"
    echo %date:~0,10% %time:~0,8% "%~1" >> %LOG_FILE_NAME%
    goto :EOF
 
:CheckUpdateType
    if NOT exist %AGENT_TMP_NEW_CERT_PATH% (
        call :Log "The path (%AGENT_TMP_NEW_CERT_PATH%) not exists. Updating failed."
        exit /b 1
    )
    if exist %AGENT_TMP_NEW_CERT_PATH%\pmca.pem (
        set UPDATE_TYPE=%UPDATE_PMCA%
        call :Log "Update pmca file only!"
        exit /b 0
    )
    if NOT exist %AGENT_TMP_NEW_CERT_PATH%\agentca.pem (
        call :Log "agentca.pem file not exists!"
        exit /b 1
    )
    if NOT exist %AGENT_TMP_NEW_CERT_PATH%\server.pem (
        call :Log "server.pem file not exists!"
        exit /b 1
    )
    if NOT exist %AGENT_TMP_NEW_CERT_PATH%\server.key (
        call :Log "server.key file not exists!"
        exit /b 1
    )
    if NOT exist %AGENT_TMP_NEW_CERT_PATH%\key.pwd (
        call :Log "key.pwd file not exists!"
        exit /b 1
    )
    call :Log "Update agentca files!"
    exit /b 0
 
:BackupOldCerts
    if exist %AGENT_TMP_OLD_CERT_PATH% (
        rd /s /q "%AGENT_TMP_OLD_CERT_PATH%"
    )
    mkdir %AGENT_TMP_OLD_CERT_PATH%
    call :Log "Create backup path!"
 
    if "%UPDATE_TYPE%" EQU "%UPDATE_PMCA%" (
        copy /y %AGENT_NGINX_CONF_PATH%\pmca.pem %AGENT_TMP_OLD_CERT_PATH% >nul
        exit /b 0
    )
    copy /y %AGENT_NGINX_CONF_PATH%\agentca.pem %AGENT_TMP_OLD_CERT_PATH% >nul
    copy /y %AGENT_NGINX_CONF_PATH%\server.pem %AGENT_TMP_OLD_CERT_PATH% >nul
    copy /y %AGENT_NGINX_CONF_PATH%\server.key %AGENT_TMP_OLD_CERT_PATH% >nul
 
    for /f %%a in ('call "%XMLCFG_EXE%" read Monitor nginx "ssl_key_password"') do set PRE_PKEY_PASSWORD=%%a
    set oldPwdFilePath=%AGENT_TMP_OLD_CERT_PATH%\key.pwd
    echo %PRE_PKEY_PASSWORD%>%oldPwdFilePath%
    set PRE_PKEY_PASSWORD=
 
    exit /b 0
 
rem 传入目标路径
:ReplaceWithNewCerts
    set targetPath=%~1
    if "%UPDATE_TYPE%" EQU "%UPDATE_PMCA%" (
        copy /y %targetPath%\pmca.pem %AGENT_NGINX_CONF_PATH%\pmca.pem >nul
        call :Log "Replace pmca successfully!"
        exit /b 0
    )
 
    rem nginx cert replace
    copy /y %targetPath%\agentca.pem %AGENT_NGINX_CONF_PATH%\agentca.pem >nul
    copy /y %targetPath%\server.pem %AGENT_NGINX_CONF_PATH%\server.pem >nul
    copy /y %targetPath%\server.key %AGENT_NGINX_CONF_PATH%\server.key >nul

    if NOT exist "%targetPath%\key.pwd" (
        call :Log "Cert password is not exist."
        exit /b 1
    )
    
    set pwdFilePath=%targetPath%\key.pwd
    set /p NEW_PKEY_PASSWORD=<%pwdFilePath%
    "%XMLCFG_EXE%" write Monitor nginx ssl_key_password "%NEW_PKEY_PASSWORD%"
    set NEW_PKEY_PASSWORD=
    call :VerifyPasswd
    if NOT %errorlevel% EQU 0 (
        echo "Failed to verify the private key password."
        call :Log "Failed to verify the private key password."
        exit /b 1
    )
    exit /b 0
 
:VerifyPasswd
    "%AGENTCLI_EXE%" verifykey | findstr /c:"failed" > nul && (
        exit /b 1
    )
    exit /b 0
 
:RollBackCert
    call :Log "Start rollback cert files."
    if exist "%AGENT_ROOT_PATH%\tmp\cert_updating" (
        call :Log "Delete file %AGENT_ROOT_PATH%\tmp\cert_updating"
        del %AGENT_ROOT_PATH%\tmp\cert_updating
    )

    if NOT exist %AGENT_TMP_OLD_CERT_PATH% (
        call :Log "The backup path: %AGENT_TMP_OLD_CERT_PATH% does not exist."
        exit /b 1
    )
    call :ReplaceWithNewCerts %AGENT_TMP_OLD_CERT_PATH%
    if NOT %errorlevel% EQU 0 (
        call :Log "Rollback failed!"
        exit /b 1
    )

    call :RestartAgent
    if NOT %errorlevel% EQU 0 (
        call :Log "Resatrt agent failed!"
        exit /b 1
    )
    call :CleanTempFiles
    exit /b 0
 
:SetCertUpdatingFlag
    call :Log "Create updating flag."
    echo. > %AGENT_ROOT_PATH%\tmp\cert_updating
    exit /b 0
 
:RestartAgent
    call %STOP_SCRIPT%
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
 
    call %START_SCRIPT%
    if NOT %errorlevel% EQU 0 (
        echo "Failed to stop the service."
        call :Log "Failed to stop the service."
        exit /b 1
    )
    call :Log "Restart DataBackup ProtectAgent successfully."
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
                taskkill /f /pid %%b >> %LOG_FILE_NAME% 2>&1
                timeout 1 /NOBREAK >> %LOG_FILE_NAME% 2>&1
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

:CleanTempFiles
    call :Log "Clean temp files!"
    set target=%AGENT_ROOT_PATH%\tmp\
    set key_word=cert_updating

    for /f "delims=" %%a in ('dir /b /s "%target%\*%key_word%*"') do (
        if exist "%%a" (
            if /i "%%~xa"=="" (
		        call :Log "delete path %%a"
                rd /s /q "%%a"
            ) else (
		        call :Log "delete file %%a"
                del /f /q "%%a"
            )
        )
    )

    if exist "%AGENT_ROOT_PATH%\tmp\pm_to_agent_status_ok" (
        call :Log "Delete file %AGENT_ROOT_PATH%\tmp\pm_to_agent_status_ok"
        del %AGENT_ROOT_PATH%\tmp\pm_to_agent_status_ok
    )

    if exist "%AGENT_ROOT_PATH%\tmp\cert_updating" (
        call :Log "Delete file %AGENT_ROOT_PATH%\tmp\cert_updating"
        del %AGENT_ROOT_PATH%\tmp\pm_to_agent_status_ok
    )
    exit /b 0