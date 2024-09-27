@echo off
setlocal EnableDelayedExpansion
rem ########################################################################
rem #
rem #  crl_update
rem #  option
rem #         help                printf usage
rem #         -i <CRL file>       import CRL file
rem #         -u <CRL file>       import CRL file, only used in upgrade Agent
rem #         -r                  restore certificate
rem #  Usage:
rem #         sh crl_update.bat -i [CRL path]
rem #         sh crl_update.bat -r
rem #         sh crl_update.bat (Enter as prompted)
rem ########################################################################
 
set CURRENT_PATH=%~dp0

rem ----------------------- path ---------------------------------------
set AGENT_MANAGER_PATH=%CURRENT_PATH%
set AGENT_ROOT_PATH=%AGENT_MANAGER_PATH%\ProtectClient-E\
set AGENT_CONF_PATH=%AGENT_ROOT_PATH%\conf\
set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\log\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\tmp\

set AGENT_NGINX_CONF_PATH=%AGENT_ROOT_PATH%\nginx\conf\

set RUNNING_CONFIG_FILE=%AGENT_CONF_PATH%\testcfg.tmp
set COMMAND_RETURN_FILE=%AGENT_TMP_PATH%\command_return.tmp

set LOG_FILE=%AGENT_LOG_PATH%\crl_update.log

rem --------------------- bat and exe file -----------------------------
set OPENSSL_EXE=%AGENT_BIN_PATH%\openssl.exe
set AGENT_START_SCRIPT=%AGENT_BIN_PATH%\agent_start.bat
set AGENT_STOP_SCRIPT=%AGENT_BIN_PATH%\agent_stop.bat

rem ----------------------- nginx file ---------------------------------
set NGINX_CONF_FILE=%AGENT_NGINX_CONF_PATH%\nginx.conf
set CA_FILE=%AGENT_NGINX_CONF_PATH%\pmca.pem
set CERT_FILE=%AGENT_NGINX_CONF_PATH%\server.pem
set CRL_NAME=server.crl
set CRL_FILE=%AGENT_NGINX_CONF_PATH%\%CRL_NAME%

@rem MODE_TYPE
set MODE_TYPE=%~1
@rem -i: Import CRL
set MODE_IMPORT=1
@rem -r: Restore certificate
set MODE_RESTORE=2
set IMPORT_CRL_FILE=%~2

@rem Flag: 0 is false, 1 is ture
set NEED_CLEAN=0
set IS_SUCC=0
set IS_UPGRADE=0

@rem CRL status
@rem IS_IMPORTED: status before update;
@rem CRL_STATUS: status after update;
set IS_IMPORTED=
set CRL_STATUS=

@rem CRL size limit is 5kb
set CRL_MAX_SIZE_5KB=5120

rem #################### Main Process ##########################
echo ####################CRL update begin##########################
net.exe session 1>NUL 2>NUL || (
    echo "Please run the script as an administrator."
    exit /b 1
)

call :CheckEnv
if NOT %errorlevel% EQU 0 (
    call :Log "Check host environment failed."
    call :ExecExit
    exit /b 1
)

call :CheckImportStatus
if NOT %errorlevel% EQU 0 (
    call :Log "Check CRL import status failed."
    call :ExecExit
    exit /b 1
)

call :CheckParam
if NOT %errorlevel% EQU 0 (
    call :Log "Check parameter failed."
    call :ExecExit
    exit /b 1
)

call :BackupFile
if NOT %errorlevel% EQU 0 (
    call :Log "Backup related files failed."
    call :ExecExit
    exit /b 1
)
set NEED_CLEAN=1

call :Log "Begin execute script."
call :Log "The current mode is [%MODE_TYPE%]."

if %MODE_TYPE% EQU %MODE_IMPORT% (
    call :CheckCRLIsImported
    if NOT !errorlevel! EQU 0 (
        call :ExecExit
        exit /b 1
    )
    call :ParseCRL
    if NOT !errorlevel! EQU 0 (
        call :ExecExit
        exit /b 1
    )
)

call :UpdateNginxConf
if NOT %errorlevel% EQU 0 (
    call :Log "Update nginx.conf failed."
    call :RollBack
    call :ExecExit
    exit /b 1
)

call :UpdateTestcfg

@rem Restart Agent for the nignx.conf modifications to take effect
call :ActivateChange
if NOT %errorlevel% EQU 0 (
    call :Log "Activate modification failed."
    echo "Execute rollback function."
    call :Log "Execute rollback function."
    call :Rollback
    call :ExecExit
    exit /b 1
)

set IS_SUCC=1
call :ExecExit
exit /b 0

rem #################### Function Process ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOG_FILE%"
    goto :EOF

:CheckEnv
    if NOT exist "%AGENT_ROOT_PATH%" (
        echo "DataBackup ProtectAgent is not detected in the current environment, failed to update CRL."
        call :Log "Check DataBackup ProtectAgent failed."
        exit /b 1
    )
    if NOT exist "%OPENSSL_EXE%" (
        echo "Can't find openssl."
        call :Log "Can't find openssl in dir[bin]."
        exit /b 1
    )
    call :Log "Check current host environment successfully."
    exit /b 0

:CheckImportStatus
    for /f "tokens=1,2 delims==" %%a in ('type "%RUNNING_CONFIG_FILE%" ^| findstr /c:"CERTIFICATE_REVOCATION="') do (
        if "%%b" EQU "" (
            echo "CRL status is wrong."
            call :Log "Please check CERTIFICATE_REVOCATION in testcfg.tmp."
            exit /b 1
        )
        set IS_IMPORTED=%%b
        exit /b 0
    )

:PrintUsage
    echo ################# Usage #################
    echo "Valid parameter, params:"
    echo "    option [file path]"
    echo "option:"
    echo "    help                printf usage"
    echo "    -i <CRL file>       import CRL file"
    echo "    -u <CRL file>       import CRL file, only used in upgrade Agent"
    echo "    -r                  restore certificate"
    echo "Usage:"
    echo "    sh crl_update.bat -i [CRL path]"
    echo "    sh crl_update.bat -r"
    echo ################# Usage #################
    exit /b 0

:CheckParam
    if "%MODE_TYPE%" EQU "" (
        call :PrintUsage
        echo.
        echo "Please input option:"
        set /p MODE_TYPE=">>"
        if "!MODE_TYPE!" EQU "-i" (
            echo "CRL file path:"
            set /p IMPORT_CRL_FILE=">>"
        ) else if "!MODE_TYPE!" EQU "-u" (
            echo "Parameter input error."
            exit /b 1
        )
    )
    if "%MODE_TYPE%" EQU "help" (
        call :PrintUsage
        echo.
        set IS_SUCC=1
        exit /b 1
    )
    if "%MODE_TYPE%" EQU "-i" (
        if NOT exist "%IMPORT_CRL_FILE%" (
            echo "Not found CRL, use help to get usage."
            exit /b 1
        )
        set MODE_TYPE=%MODE_IMPORT%
        set CRL_STATUS=1
        echo "Curren option is import CRL."
        exit /b 0
    )
    if "%MODE_TYPE%" EQU "-r" (
        if NOT "%IS_IMPORTED%" EQU "1" (
            echo "There is no imported CRL, restore operation is illegal"
            exit /b 1
        )
        set MODE_TYPE=%MODE_RESTORE%
        set CRL_STATUS=0
        echo "Curren option is restore certificate."
        exit /b 0
    )
    if "%MODE_TYPE%" EQU "-u" (
        if NOT exist "%IMPORT_CRL_FILE%" (
            echo "Not found CRL, use help to get usage."
            exit /b 1
        )
        for %%a in (%IMPORT_CRL_FILE%) do (
            if NOT "%%~dpa" EQU "C:\DataBackup\AgentUpgrade\Old\ProtectClient-E\nginx\conf\" (
                echo "Crl file must be in the C:\DataBackup\AgentUpgrade\Old\ProtectClient-E\nginx\conf\."
                exit /b 1
            )
        )
        set MODE_TYPE=%MODE_IMPORT%
        set IS_UPGRADE=1
        set CRL_STATUS=1
        exit /b 0
    )
    echo "Invalid argument, use help to get usage."
    exit /b 1

:CheckCRLIsImported
    if %IS_UPGRADE% EQU 1 (
        exit /b 0
    )
    if %IS_IMPORTED% EQU 0 (
        echo "There is no CRL file imported."
        call :Log "There is no imported CRL file."
        exit /b 0
    )
    if %IS_IMPORTED% EQU 1 (
        for %%a in (1 2 3) do (
            echo "The certificate is currently imported, whether to overwrite it?[y/n]."
            set /p choose="try %%a/3 >>"
            if "!choose!" EQU "y" (
                echo "User choose to overwrite imported CRL file."
                call :Log "Overwrite imported CRL file."
                exit /b 0
            )
            if "!choose!" EQU "n" (
                echo "User cancel to import CRL file."
                exit /b 1
            )
            echo "Please enter y (YES) or n (NO), and default is n."
        )
        echo "The number of incorrect inputs exceeds 3. default is cancel import."
        call :Log "The number of incorrect inputs exceeds 3. default is cancel import."
        exit /b 1
    )

:ParseCRL
    @rem Check if it is a CRL file
    "%OPENSSL_EXE%" crl -in "%IMPORT_CRL_FILE%" -noout -text 2>&1 | findstr /c:"unable to load CRL" > nul && (
        echo "Input file is not a CRL."
        call :Log "Load CRL file failed."
        exit /b 1
    )

    @rem No certificate have been revoked in CRL
    "%OPENSSL_EXE%" crl -in "%IMPORT_CRL_FILE%" -noout -text 2>&1 | findstr /c:"No Revoked Certificates" > nul && (
        echo "No certificate have been revoked in CRL."
        call :Log "Input CRL is NULL."
        exit /b 1
    )

    @rem Check CRL file size
    for %%a in ("%IMPORT_CRL_FILE%") do set key_words=%%~za
    if %key_words% GTR %CRL_MAX_SIZE_5KB% (
        echo "File size missed limit."
        call :Log "CRL file size exceeded."
        exit /b 1
    )

    @rem Check CA chain and CRL expired
    "%OPENSSL_EXE%" crl -in "%IMPORT_CRL_FILE%" -CAfile "%CA_FILE%" -noout -verify 2>&1 | findstr /c:"verify OK" > nul
    if NOT %errorlevel% EQU 0 (
        echo "The imported CRL does not match CA certificate."
        call :Log "CRL does not match CA certificate."
        exit /b 1
    )

    @rem Check CRL update
    "%OPENSSL_EXE%" verify -crl_check -CRLfile "%IMPORT_CRL_FILE%" -CAfile "%CA_FILE%" "%CERT_FILE%" 2>&1 | findstr /c:"CRL has expired" > nul && (
        echo "The valid time in the input CRL is incorrect."
        call :Log "Check CRL expiration date failed."
        exit /b 1
    )

    copy /y "%IMPORT_CRL_FILE%" "%CRL_FILE%" > nul
    exit /b 0

@rem back file(nginx.conf testcfg.tmp server.crl)
:BackupFile
    copy /y "%RUNNING_CONFIG_FILE%" "%RUNNING_CONFIG_FILE%.bak" > nul 2>&1
    copy /y "%NGINX_CONF_FILE%" "%NGINX_CONF_FILE%.bak" > nul 2>&1
    if exist "%CRL_FILE%" (
        copy /y "%CRL_FILE%" "%CRL_FILE%.bak" > nul 2>&1
    )
    exit /b 0

:UpdateNginxConf
    if NOT exist "%NGINX_CONF_FILE%" (
        echo "Can't find nginx.conf."
        call :Log "Can't find nginx.conf, DataBackup ProtectAgent is damaged."
        exit /b 1
    )

    @rem Delete the ssl_crl line in the nginx.conf
    type "%NGINX_CONF_FILE%" | findstr -v /c:"ssl_crl " > "%COMMAND_RETURN_FILE%"
    move /y "%COMMAND_RETURN_FILE%" "%NGINX_CONF_FILE%" > nul
    if %MODE_TYPE% EQU %MODE_RESTORE% (
        echo "Update nginx.conf successfully."
        call :Log "Update nginx.conf successfully, in restore certificate."
        exit /b 0
    )

    @rem Add the ssl_crl line in the nginx.conf, in import CRL mode
    for /f "tokens=1* delims=:" %%a in ('findstr /n ".*" "%NGINX_CONF_FILE%"') do (
        @rem Avoid ignoring blank lines
        if "%%b" EQU "" (
            @rem Avoid ehco error due to line consisting of spaces  
            echo.>>"%COMMAND_RETURN_FILE%"
        )
        for /f "tokens=*" %%A in ("%%b") do (
            if "%%A" EQU "" (
                echo.>>"%COMMAND_RETURN_FILE%"
            ) else (
                echo %%b>>"%COMMAND_RETURN_FILE%"
                @rem Use spaces to keep formatting aligned
                echo %%b | findstr /c:"ssl_certificate_key " >nul  && echo         ssl_crl %CRL_NAME%;>>"%COMMAND_RETURN_FILE%"
            )
        )
    )
    move /y "%COMMAND_RETURN_FILE%" "%NGINX_CONF_FILE%" > nul
    for /f "" %%a in ('type "%NGINX_CONF_FILE%" ^| findstr /c:"ssl_crl %CRL_NAME%"') do set key_words=%%a
    if "%key_words%" EQU "" (
        echo "Update Nginx.Conf failed."
        call :Log "Update Nginx.Conf failed."
        exit /b 1
    )
    echo "Update nginx.conf successfully."
    call :Log "Update nginx.conf successfully, in import CRL."
    exit /b 0

:UpdateTestcfg
    for /f "tokens=1* delims==" %%a in ('type "%RUNNING_CONFIG_FILE%"') do (
        if "%%a" EQU "CERTIFICATE_REVOCATION" (
            if %MODE_TYPE% EQU %MODE_IMPORT% (
                echo %%a=!CRL_STATUS!>>"%COMMAND_RETURN_FILE%"
            ) else (
                echo %%a=!CRL_STATUS!>>"%COMMAND_RETURN_FILE%"
            )
        ) else (
            echo %%a=%%b>>"%COMMAND_RETURN_FILE%"
        )
    )
    move /y "%COMMAND_RETURN_FILE%" "%RUNNING_CONFIG_FILE%" > nul
    echo "Update testcfg.tmp completed."
    call :Log "Update testcfg.tmp completed."
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
			taskkill /f /pid %%b 1>nul 2>nul
			timeout 1 /NOBREAK > nul
            set /a LOOP_COUNT+=1
			goto :loopcheckprocess
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
        call :Log "Failed to stop the service."
        exit /b 1
    )
    call :CheckProcessStatus
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec CheckProcessStatus failed."
        exit /b 1
    )
    call %AGENT_START_SCRIPT%
    if NOT %errorlevel% EQU 0 (
        call :Log "Failed to stop the service."
        exit /b 1
    )
    call :Log "Restart DataBackup ProtectAgent successfully."
    exit /b 0

:ActivateChange
    if %IS_UPGRADE% EQU 1 (
        call :RestartAgent
        if NOT !errorlevel! EQU 0 (
            call :Log "Restart DataBackup ProtectAgent failed, in upgrade Agent."
            exit /b 1
        )
        call :Log "Restart DataBackup ProtectAgent successfully, in upgrade Agent." 
        exit /b 0
    )
    echo "Restart the DataBackup ProtectAgent to make the changes take effect , whether restart DataBackup ProtectAgent immediately?[y/n]"
    for %%a in (1 2 3) do (
        set /p choose="try %%a/3 >>"
        if "!choose!" EQU "y" (
            echo "Restart DataBackup ProtectAgent."
            call :Log "Restart DataBackup ProtectAgent."
            call :RestartAgent
            if NOT !errorlevel! EQU 0 (
                echo "Restart DataBackup ProtectAgent failed."
                call :Log "Restart DataBackup ProtectAgent failed, in manual excute script."
                exit /b 1
            )
            echo "Restart DataBackup ProtectAgent successfully."
            exit /b 0
        )
        if "!choose!" EQU "n" (
            echo "User cancel to restart DataBackup ProtectAgent."
            call :Log "User cancel to restart DataBackup ProtectAgent."
            exit /b 1
        )
        echo "Please enter y (YES) or n (NO), and default is n."
    )
    echo "The number of incorrect inputs exceeds 3."
    call :Log "The number of incorrect inputs exceeds 3."
    exit /b 1

:Rollback
    copy /y "%NGINX_CONF_FILE%.bak" "%NGINX_CONF_FILE%" > nul 2>&1
    copy /y "%RUNNING_CONFIG_FILE%.bak" "%RUNNING_CONFIG_FILE%" > nul 2>&1
    if %MODE_TYPE% EQU %MODE_IMPORT% (
        copy /y "%CRL_FILE%.bak" "%CRL_FILE%" > nul 2>&1
    )
    exit /b 0

:ClearSource
    del "%RUNNING_CONFIG_FILE%.bak" "%NGINX_CONF_FILE%.bak" > nul 2>&1
    if exist "%CRL_FILE%.bak" (
        del "%CRL_FILE%.bak" > nul 2>&1
    )
    exit /b 0

:ExecExit
    call :Log "crl_update.bat execute ExecExit"
    if %NEED_CLEAN% EQU 1 (
        call :ClearSource
    )
    if %IS_SUCC% EQU 1 (
        echo ####################Execute  succeed##########################
        call :Log "####################Execute  succeed##########################"
    ) else (
	    echo #################### Execute failed ##########################
        call :Log "#################### Execute failed ##########################"
    ) 
    call :Log "Exit crl_update.bat"
    if %IS_UPGRADE% EQU 0 (
        echo "The process is complete, press any key to exit"
        pause
    )
    endlocal
exit /b 0