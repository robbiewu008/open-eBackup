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

rem Database path to be upgraded
set SQLITE_BACKUP_FILE_DIR=%~1
rem Upgrate the database path in the package
set SQLITE_UPGRADE_FILE_DIR=%~2

set CURRENT_PATH=%~dp0
set INSTALLTION_PACKAGE_PATH=%CURRENT_PATH%\..\
set TEMP_AGENT_LOG_PATH=%INSTALLTION_PACKAGE_PATH%\log\
set TEMP_AGENT_TMP_PATH=%INSTALLTION_PACKAGE_PATH%\tmp\
set LOGFILE_PATH=%TEMP_AGENT_LOG_PATH%\agent_upgrade_sqlite.log

set UPGRADE_VERSION=
set UPGRADE_SUPPORT_VERSION_ARRAY=
set OLD_VERSION=
set LOOP_COUNT=0
set NEED_UPGRADE=1
set IS_SUCC=0

if exist "%LOGFILE_PATH%" (
    del "%LOGFILE_PATH%"
)

rem #################### Main Process ##########################
echo ####################agent_upgrade_sqlite begin##########################
call :Log "####################agent_upgrade_sqlite begin##########################"

rem Checking the upgrade information
call :Log "Exec GetUpgradeVersion function."
call :GetUpgradeVersion
if NOT %errorlevel% EQU 0 (
    call :Log "Exec GetUpgradeVersion function failed."
    call :ExecExit
    exit /b 1
)

rem Checking the current information
call :Log "Exec GetCurrentVersion function."
call :GetCurrentVersion
if NOT %errorlevel% EQU 0 (
    call :Log "Exec GetCurrentVersion function failed."
    call :ExecExit
    exit /b 1
)

rem Verify the upgrade version
call :Log "Exec VerifyUpgradeVersion function."
call :VerifyUpgradeVersion
if NOT %errorlevel% EQU 0 (
    call :Log "Exec VerifyUpgradeVersion function failed."
    call :ExecExit
    exit /b 1
)

rem run RunUpgradeSql
if %NEED_UPGRADE% EQU 1 (
    call :Log "Exec RunUpgradeSql function."
    call :RunUpgradeSql
    if NOT %errorlevel% EQU 0 (
        call :Log "Exec RunUpgradeSql function failed."
        call :ExecExit
        exit /b 1
    )
) else (
    call :Log "No need to exec upgrade.sql."
)

set IS_SUCC=1
call :ExecExit
exit /b 0

rem #################### Function ##########################

:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
goto :EOF

:GetUpgradeVersion
    call :Log "Check upgrade information begin."
    if NOT exist "%SQLITE_UPGRADE_FILE_DIR%" (
        call :Log "The upgrade directory cannot be found."
        exit /b 1
    )
    if NOT exist "%SQLITE_UPGRADE_FILE_DIR%\upgrade\upgrade.sql" (
        call :Log "The upgrade sql file cannot be found."
        exit /b 1
    )
    if NOT exist "%SQLITE_UPGRADE_FILE_DIR%\upgrade\attribute.conf" (
        call :Log "The upgrade attribute file cannot be found."
        exit /b 1
    )
    call :Log "Cheack related files successfully."

    for /f "delims== tokens=1,2" %%a in ('type "%SQLITE_UPGRADE_FILE_DIR%\upgrade\attribute.conf"') do (
        if %%a EQU version (
            set UPGRADE_VERSION=%%b
        ) else if %%a EQU support_version (
            set UPGRADE_SUPPORT_VERSION_ARRAY=%%b
        )
    )
    echo "New version=%UPGRADE_VERSION%, support_version=%UPGRADE_SUPPORT_VERSION_ARRAY%."
    call :Log "New version=%UPGRADE_VERSION%, support_version=%UPGRADE_SUPPORT_VERSION_ARRAY%."

    call :Log "Check upgrade information end."
    exit /b 0
goto :EOF

:GetCurrentVersion
    call :Log "Check old information begin."

    if NOT exist "%SQLITE_BACKUP_FILE_DIR%" (
        call :Log "The backup upgrade directory cannot be found."
        exit /b 1
    )
    if NOT exist "%SQLITE_BACKUP_FILE_DIR%\upgrade\upgrade.sql" (
        call :Log "The upgrade sql file cannot be found."
        exit /b 1
    )
    if NOT exist "%SQLITE_BACKUP_FILE_DIR%\upgrade\attribute.conf" (
        call :Log "The upgrade attribute file cannot be found."
        exit /b 1
    )
    call :Log "Cheack related files successfully."

    for /f "delims== tokens=1,2" %%a in ('type "%SQLITE_BACKUP_FILE_DIR%\upgrade\attribute.conf"') do (
        if %%a EQU version (
            set OLD_VERSION=%%b
        )
    )
    echo "Old version=%OLD_VERSION%."
    call :Log "Old version=%OLD_VERSION%."

    call :Log "Check old information end."
goto :EOF

:VerifyUpgradeVersion
    call :Log "Verify version begin."
    if "%UPGRADE_VERSION%" EQU "" (
        call :Log "Upgrade configuration file content error."
        exit /b 1
    ) else if "%OLD_VERSION%" EQU "" (
        call :Log "Current configuration file content error."
        exit /b 1
    )

    if %OLD_VERSION% EQU %UPGRADE_VERSION% (
        set NEED_UPGRADE=0
        call :Log "Upgrade with the same version."
        exit /b 0
    )

    if "%UPGRADE_SUPPORT_VERSION_ARRAY%" EQU "" (
        call :Log "Upgrade support version is empty."
        exit /b 1
    )

    :loopchecksupport
    set /a LOOP_COUNT+=1
    for "delims=, tokens=%LOOP_COUNT%" %%a in ("%UPGRADE_SUPPORT_VERSION_ARRAY%") do (
        if %%a EQU %OLD_VERSION% (
            call :Log "Support version matching successfully."
            exit /b 0
        )
        if "%%a" EQU "" (
            call :Log "Support version matching failed."
            exit /b 1
        ) else (
            goto :loopchecksupport
        )
    )
goto :EOF

:RunUpgradeSql
    call :Log "Start upgrade."
    for /f "delims=" %%a in ("%SQLITE_UPGRADE_FILE_DIR%\upgrade\upgrade.sql") do (
        sqlite3 "%SQLITE_BACKUP_FILE_DIR%\AgentDB.db" "%%a"
        if NOT !errorlevel! EQU 0 (
            call :Log "Failed exec %SQLITE_UPGRADE_FILE_DIR%\upgrade\upgrade.sql."
            rd /s /q  "%SQLITE_BACKUP_FILE_DIR%" > nul
            exit /b 1
        )
    )
    copy /y "%SQLITE_BACKUP_FILE_DIR%\AgentDB.db" "%SQLITE_UPGRADE_FILE_DIR%\AgentDB.db" >nul
    call :Log "The sql is executed successfully."
    exit /b 0
goto :EOF

:ExecExit
    call :Log "agent_upgrade_sqlite.bat exec ExecExit"
    if %IS_SUCC% EQU 1 (
        call :Log "Upgrade Sqllite successfully"
        echo ####################Agent_upgrade_sqlite succ##########################
        call :Log "####################Agent_upgrade_sqlite succ##########################" 
    ) else (
        call :Log "Upgrade Sqllite failed"
        echo ####################Agent_upgrade_sqlite failed##########################
        call :Log "####################Agent_upgrade_sqlite failed##########################" 
    )
    endlocal
goto :EOF