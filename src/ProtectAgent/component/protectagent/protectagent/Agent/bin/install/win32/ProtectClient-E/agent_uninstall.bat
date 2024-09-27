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

rem #######################################################
rem #
rem #  upgrade: %~1=/q  push upgrade: %~1=/r
rem #   
rem #  %~2: "1":not del working user; others del
rem #
rem #######################################################

setlocal EnableDelayedExpansion

set CURRENT_PATH=%~dp0
set AGENT_LOG_PATH=%CURRENT_PATH%..\log\
set LOGFILE_PATH=%AGENT_LOG_PATH%agent_uninstall.log
set SESERV_LOGON_RIGHT_FILE=%CURRENT_PATH%..\tmp\logonrightinfo
set UNINSTALL_TMP_FILE=%CURRENT_PATH%..\tmp\uninstalltmpinfo
set SESERV_LOGON_RIGHT_TMP_DB=%CURRENT_PATH%..\tmp\logonright.sdb
set /a NUMBER=0
set NEED_LOG=1
set UNINSTALL_RESULT=0
set CMD_PAUSE=pause
set PING_FLAG=/q

set AGENT_USER=rdadmin
set USER_DEL=0
set USER_DEL_OPER=0
set EXEC_FLAG=%~1
set PRODUCT_AGENT_NAME=DataBackup ProtectAgent
if "eBackup" == "%~1" (
    set PRODUCT_AGENT_NAME=Cloud Server Backup Service Agent
)

call :Log "########################Begin uninstall Agent########################"

if "%EXEC_FLAG%" == "/q" (
    set CMD_PAUSE=
    set USER_DEL_OPER=%~2
    call :uninstallagent
    set RESULT_NO=!errorlevel!
) else if "%EXEC_FLAG%" == "/r" (
    set CMD_PAUSE=
    set USER_DEL_OPER=%~2
    set PING_FLAG=/r
    call :uninstallagent
    set RESULT_NO=!errorlevel!
) else (
    echo You are about to uninstall the DataBackup ProtectAgent. This operation will stop the ProtectAgent service and the protected environment will no longer be protected.
    echo.
    echo Suggestion: Check whether the user-defined script needs to be backed up. The user-defined script will be deleted during the uninstallation.
    echo Log in to the OceanProtect management page to clear the resources corresponding to the host.
    echo.
    
    call :getinput
    call :uninstallagent
    set RESULT_NO=!errorlevel!
)

echo.
if not "!RESULT_NO!" == "0" (
    echo %PRODUCT_AGENT_NAME% was uninstalled failed.
    call :Log "%PRODUCT_AGENT_NAME% was uninstalled failed."
    %CMD_PAUSE%
    exit /b 1
)

echo %PRODUCT_AGENT_NAME% was uninstalled successfully.
echo %PRODUCT_AGENT_NAME% has been uninstalled successfully, the applications on the host are no longer protected.
call :Log "%PRODUCT_AGENT_NAME% was uninstalled successfully."
echo.
if not "%EXEC_FLAG%" == "%PING_FLAG%" (
    echo Please remove the installation folders of %PRODUCT_AGENT_NAME%.
    timeout /T 3 /NOBREAK >nul
)

exit /b 0

:getinput
    set IFLAG=1
    echo Are you sure you want to uninstall %PRODUCT_AGENT_NAME%? ^(y/n, default:n^):
    set /p UNINSTALL=">>"
    
    if "!UNINSTALL!" == "" (
        call :gotoexit
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
            call :gotoexit
        )
    )
    
    if "!UNINSTALL!" == "n" (
        call :gotoexit
    )
    
    goto :EOF
    
:uninstallagent
    echo Begin to uninstall %PRODUCT_AGENT_NAME%...
    echo.
    
    if not exist "%CURRENT_PATH%\uninstall-provider.cmd" (
        echo File uninstall-provider.cmd is not exist.
        call :Log "File uninstall-provider.cmd is not exist, then exit 1."
        exit /b 1
    )
    
    if not exist "%CURRENT_PATH%\winservice.exe" (
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
    
    call :unregistprovider
    if not "!errorlevel!" == "0" (exit /b 1)
    
    if not "!USER_DEL!" == "0" (
        if not "1" == "!USER_DEL_OPER!" (
            call :deleteworkinguser !AGENT_USER!
            if not "!errorlevel!" == "0" (exit /b 1)
        )
    )
    
    exit /b 0
    
:unregistprovider
    set SERVICE_NAME=rdprovider
    set SERVER_QUERY=
    
    for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set SERVER_QUERY=%%b)
    if "!SERVER_QUERY!" == "" (
        echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully.
        call :Log "Process !SERVICE_NAME! of %PRODUCT_AGENT_NAME% is not exist, no need uninstall."
        exit /b 0
    )
    
    set USER_DEL=1
    call "%CURRENT_PATH%\uninstall-provider.cmd">>"%LOGFILE_PATH%" 2>&1
    ping -n 3 127.0>nul
    set OPER_RESULT=
    for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set OPER_RESULT=%%b)
    if not "!OPER_RESULT!" == "" (
        echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled failed.
        call :Log "Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled failed."
        exit /b 1
    )
    
    echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully.
    call :Log "Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully."    
    exit /b 0

rem -----------------
rem %~1:SERVICE_NAME
rem %~2:SERVICE_PARAM
rem -----------------    
:unregistServices
    set SERVER_QUERY=
    set SERVICE_NAME=%~1
    set SERVICE_PARAM=%~2
    for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set SERVER_QUERY=%%b)
    if "!SERVER_QUERY!" == "" (
        echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully.
        call :Log "Process !SERVICE_NAME! of %PRODUCT_AGENT_NAME% is not exist, no need uninstall."
        exit /b 0
    )

    set USER_DEL=1
    sc stop !SERVICE_NAME! >> "%AGENT_LOG_PATH%\agent_uninstall.log"
    
    call "%CURRENT_PATH%\winservice.exe" !SERVICE_PARAM! uninstall
    if not "!errorlevel!" == "0" (
        echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled failed.
        call :Log "Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% failed."
        exit /b 1
    ) else (
        timeout /T 3 /NOBREAK >nul
        
        set OPER_RESULT=
        for /f "tokens=1,2 delims=:" %%a in ('2^>nul sc query !SERVICE_NAME! ^| findstr SERVICE_NAME') do (set OPER_RESULT=%%b)
        if "!OPER_RESULT!" == "" (
            echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled successfully.
            call :Log "Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% successfully."
            exit /b 0
        ) else (
            echo Service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% was uninstalled failed.
            call :Log "Uninstall service !SERVICE_NAME! of %PRODUCT_AGENT_NAME% failed."
            exit /b 1
        )
    )
    
    exit /b 0

rem ---------------
rem %~1: Agent_user
rem --------------- 
:deleteworkinguser
    set WORKING_USER=%~1
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
    
    net user !WORKING_USER! /delete >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Delete user !WORKING_USER! failed."
        echo User !WORKING_USER! of %PRODUCT_AGENT_NAME% was Deleted failed.
        exit /b 1
    )
    
    call :Log "Delete the working user !WORKING_USER! infos."
    for /f "tokens=*" %%a in ('2^>nul reg query "hklm\software\microsoft\windows nt\currentversion\profilelist" ^| findstr /i "s-1-5-21"') do (call :deleteuserinfos "%%a")
    
    echo User !WORKING_USER! of %PRODUCT_AGENT_NAME% was Deleted successfully.
    call :Log "Delete user !WORKING_USER! succ."
    exit /b 0

:deleteuserinfos
    set USER_REG_PATH=
    for /f "tokens=1,2,*" %%a in ('2^>nul reg query "%~1" /v ProfileImagePath') do (set USER_REG_PATH=%%c)
    
    if "!USER_REG_PATH!" == "" (
        call :Log "Query user home director failed."
        exit /b 1
    )
    
    for /f "tokens=3 delims=\" %%e in ('echo.!USER_REG_PATH!') do (set USER_REG=%%e)
    for /f "tokens=1 delims=." %%f in ('echo.!USER_REG!') do (set USER_REG_PARSE=%%f)
    
    rem other users, do not deal
    if not "!USER_REG_PARSE!" == "!WORKING_USER!" (exit /b 0)
    
    rem clear regeditor
    reg delete "%~1" /f 1>nul 2>>"%LOGFILE_PATH%"
    
    rem delete home director
    if exist "!USER_REG_PATH!" (rd /s /q "!USER_REG_PATH!") 1>nul 2>>"%LOGFILE_PATH%"
    
    rem check infos
    reg query "%~1" 1>nul 2>&1
    if "!errorlevel!" == "0" (
        call :Log "The regeditor of %~1 delete failed."
        exit /b 1
    )
    
    if exist "!USER_REG_PATH!" (
        call :Log "Delete the !USER_REG_PATH! failed."
        exit /b 1
    )
    
    call :Log "Clear working user infos succ."
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

    secedit /export /cfg "!UNINSTALL_TMP_FILE!" >>"%LOGFILE_PATH%" 2>&1
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
    secedit /import /db "!SESERV_LOGON_RIGHT_TMP_DB!" /cfg "!SESERV_LOGON_RIGHT_FILE!" >>"%LOGFILE_PATH%" 2>&1
    if not "!errorlevel!" == "0" (
        call :Log "Import the !SESERV_LOGON_RIGHT_FILE! to the !SESERV_LOGON_RIGHT_TMP_DB! failed."
        if exist "!SESERV_LOGON_RIGHT_FILE!"  del /f /q "!SESERV_LOGON_RIGHT_FILE!"
        exit /b 1
    )
    
    if exist "!SESERV_LOGON_RIGHT_FILE!"    del /f /q "!SESERV_LOGON_RIGHT_FILE!"
    
    rem add user seservicelogonright Log on as a service
    secedit /configure /db "!SESERV_LOGON_RIGHT_TMP_DB!" >>"%LOGFILE_PATH%" 2>&1
    if not "!errorlevel!" == "0" (
        call :Log "Add user !WORKING_USER! seservicelogonright to log on as a service failed."
        if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
        exit /b 1
    )
    
    if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
    call :Log "Remove user !WORKING_USER! seservicelogonright to log on as a service succ."
    
    exit /b 0

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> "%LOGFILE_PATH%"
    )
    
    call "%CURRENT_PATH%\agent_func.bat" "%LOGFILE_PATH%"
    
    goto :EOF
    
:gotoexit
    echo The uninstallation of %PRODUCT_AGENT_NAME% will be stopped.
    %CMD_PAUSE%
    exit 1
  
endlocal