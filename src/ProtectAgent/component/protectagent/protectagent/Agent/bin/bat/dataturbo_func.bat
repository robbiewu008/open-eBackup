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
setlocal EnableDelayedExpansion
set FUNC_NAME=%~1
set LOG_FILE=%~2
set AGENT_INSTALL_PATH=%~3
set MODE=%~4
set WIN_SYSTEM_DISK=%WINDIR:~0,1%

set ZIP_TOOL_PATH="%AGENT_INSTALL_PATH%third_party_software\7ZIP\7z.exe"
set CURRENT_PATH=%~dp0
set DEFAULT_INSTALL_DIR=C:\Program Files
if not "%WIN_SYSTEM_DISK%" == "" (
    set DEFAULT_INSTALL_DIR=%WIN_SYSTEM_DISK%:\Program Files
)
set INSTALLED_BY_AGENT=%DEFAULT_INSTALL_DIR%\oceanstor\dataturbo\.InstallByAgent
set REG_KEY_IN_USER=HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\dataturbo
set REG_KEY_IN_MACHINE=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\dataturbo
set REG_VAR_UNINST=UninstallString
set DEFAULT_DATATURBO_USER=dataturbo
set DEFAULT_CLI_USER=dataturbo_mgmt
rem This password just used for create dataturbo cli user
set DATATURBO_USER_PASSWORD_LEN=20
set DATATURBO_PKG="%AGENT_INSTALL_PATH%third_party_software\dataturbo-windows.zip"
set INFO_TMP_FILE=%AGENT_ROOT_PATH%\log\dataturboInfo.log
set DATATURBO_EXIT_CODE_OS_NOT_SUPPORT=99
set DATATURBO_EXIT_CODE_LACK_RESOURCE=100

call :!FUNC_NAME!
exit /b !errorlevel!
goto :EOF

:CheckDataTurboInstalled
    rem ********************Begin check Dataturbo installed********************
    rem query dataturbo service
    sc query dataturbo >nul 2>nul
    if !errorlevel! EQU 0 (
        exit /b 0
    )
    exit /b 1
goto :EOF

:InstallDataTurbo
    call :CheckDataTurboInstalled
    if !errorlevel! EQU 0 (
        call :LogAndPrint "The dataturbo has been installed, need uninstall it."
        exit /b 1
    )

    SET DATATURBO_PATH=
    call :PrepareInstallPackage DATATURBO_PATH
    if NOT !errorlevel! EQU 0 (
        call :LogAndPrint "Prepare dataturbo package failed."
        exit /b 1
    )
    call :LogAndPrint "Begin install dataturbo."
    if exist %INFO_TMP_FILE% (
        del /q /f %INFO_TMP_FILE%
    )
    call :GenPwd
    call "%DATATURBO_PATH%\install.bat" /INSTALL_DIR "!DEFAULT_INSTALL_DIR!" /DATATURBO_USER "!DEFAULT_DATATURBO_USER!" /CLI_USER "!DEFAULT_CLI_USER!" /CLI_PASSWORD "!DEFAULT_CLI_PWD!" /NON_INTERACT "y" >%INFO_TMP_FILE%
    set status=!errorlevel!
    if !status! EQU 1 (
        call :LogAndPrint "Install dataturbo failed."
        call :UninstallDataTurbo
        exit /b 1
    )
    if !status! EQU !DATATURBO_EXIT_CODE_OS_NOT_SUPPORT! (
        @REM 系统不支持dataturbo，跳过安装
        call :LogAndPrint "os version is not support dataturbo."
        exit /b 1
    )
    if !status! EQU !DATATURBO_EXIT_CODE_LACK_RESOURCE! (
        @REM 资源不足，跳过安装
        call :LogAndPrint "lack of resources of installing dataturbo."
        exit /b 1
    )
    type %INFO_TMP_FILE% >> "%LOG_FILE%"
    if not exist "%INSTALLED_BY_AGENT%" (md "%INSTALLED_BY_AGENT%" && attrib +h "%INSTALLED_BY_AGENT%")

    set whiteListFile=%DEFAULT_INSTALL_DIR%\oceanstor\dataturbo\conf\whitelist
    set systemSid=S-1-5-18
    if not exist !whiteListFile! (
        call :LogAndPrint "whitelist not exist."
        call :UninstallDataTurbo
        exit /b 1
    )
    echo %systemSid%>> !whiteListFile!
    call :LogAndPrint "add system user to whitelist success."

    if defined AGENT_NEWPKG_BIN_PATH (
        call "%AGENT_NEWPKG_BIN_PATH%\agent_start.bat" dataturbo
    ) else (
        call "%AGENT_BIN_PATH%\agent_start.bat" dataturbo
    )
    if NOT !errorlevel! EQU 0 (
        call :LogAndPrint "Service dataturbo start failed."
        call :UninstallDataTurbo
        exit /b 1
    )

    call :LogAndPrint "Install dataturbo successfully."
    exit /b 0
goto :EOF

:GenPwd
    set "up_letter=ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    set "low_letter=abcdefghijklmnopqrstuvwxyz"
    set "digit=1234567890"
    set special="$@#*_-+?"

    set "password="

    set /a up_letter_radom=!random! %% 3 + 1
    set /a low_letter_radom=!random! %% 3 + 1
    set /a digit_radom=!random! %% 3 + 1
    set /a special_radom=%DATATURBO_USER_PASSWORD_LEN%-!digit_radom!-!up_letter_radom!-!low_letter_radom!

    call :DoGenPwd !up_letter_radom! %up_letter% 26
    call :DoGenPwd !low_letter_radom! %low_letter% 26
    call :DoGenPwd !digit_radom! %digit% 10
    call :DoGenPwd !special_radom! %special% 8
    set DEFAULT_CLI_PWD=!password!
    exit /b 0

:DoGenPwd
    for /l %%i in (1,1,%~1) do (
        set randomrange=%~3
        set /a index=!random! %% !randomrange! + 1
        set randomchar="%~2"
        call set "zchar=%%randomchar:~!index!,1%%"
        set "password=!password!!zchar!"
    )
    exit /b 0

:UninstallDataTurbo
    call :CheckDataTurboInstalled || (
        exit /b 0
    )
    if not exist "%INSTALLED_BY_AGENT%" (
        call :LogAndPrint "The dataturbo was not installed by agent, will not be uninstall."
        exit /b 0
    )
    rem ********************Begin uninstall dataturbo********************
    set UNINST_CMD=
    call :QueryUninstallBat UNINST_CMD
    if "!UNINST_CMD!" == "" (
        call :LogAndPrint "Query dataturbo uninstall script failed."
        exit /b 1
    )
    set UNINST_CMD="!UNINST_CMD:.bat=.bat"!
    call :LogAndPrint "Begin uninstall dataturbo."
    echo y|call %UNINST_CMD%
    if !errorlevel! EQU 1 (
        call :LogAndPrint "Uninstalled dataturbo failed."
        exit /b 1
    )
    call :LogAndPrint "Uninstalled dataturbo successfully ."
    exit /b 0
goto :EOF

:QueryUninstallBat
    rem query from registry
    for /f "tokens=2,*" %%i in ('reg query "%REG_KEY_IN_MACHINE%" /v "%REG_VAR_UNINST%"') do (
        set "%~1=%%j"
        exit /b 0
    )
    for /f "tokens=2,*" %%i in ('reg query "%REG_KEY_IN_USER%" /v "%REG_VAR_UNINST%"') do (
        set "%~1=%%j"
        exit /b 0
    )
    exit /b 0
goto :EOF

:PrepareInstallPackage
    if not exist %DATATURBO_PKG% (
        call :LogAndPrint "Missing dataturbo install package."
        exit /b 1
    )
    rem ********begin unzip dataturbo package********
    "%ZIP_TOOL_PATH%" x -tzip -y "%DATATURBO_PKG%" -o"%CURRENT_PATH%" -mx=9 >nul 2>nul
    if !errorlevel! EQU 1 (
        call :LogAndPrint "unzip dataturbo package error."
        exit /b 1
    )
    SET UNZIP_PATH=%~1
    for /f "usebackq" %%s in (`dir /S /B %CURRENT_PATH%\OceanStor_DataTurbo_*_Windows*`) do (
        SET %UNZIP_PATH%=%%s
        exit /b 0
    )
    call :LogAndPrint "The directory OceanStor_DataTurbo_*_Windows is not exists."
    exit /b 1
goto :EOF

:UpgradeDataTurbo
    call :CheckDataTurboInstalled
    if NOT !errorlevel! EQU 0 (
        call :LogAndPrint "Can't execute upgrade, dataturbo is not installed."
        exit /b 1
    )
    rem ********check the dataturbo mount directory********
    dataturbo show mount_dir |findstr mnt >nul 2>nul
    if "!errorlevel!"=="0" (
        call :LogAndPrint "There are dataturbo mount directory left on the agent.After the upgrade, it may cause an emergency alert for dataturbo mount failure, but not affect business functions."
    )
    call "%AGENT_NEWPKG_BIN_PATH%\agent_stop.bat" dataturbo
    if NOT !errorlevel! EQU 0 (
        call :LogAndPrint "Service dataturbo stop failed."
        exit /b 1
    )
    SET DATATURBO_PATH=
    call :PrepareInstallPackage DATATURBO_PATH
    if NOT !errorlevel! EQU 0 (
        call :LogAndPrint "Prepare dataturbo package failed."
        exit /b 1
    )
    if exist %INFO_TMP_FILE% (
        del /q /f %INFO_TMP_FILE% >nul 2>nul
    )
    call :LogAndPrint "Begin upgrade the dataturbo."
    echo y|call "%DATATURBO_PATH%\upgrade.bat" > %INFO_TMP_FILE%
    if NOT !errorlevel! EQU 0 (
        findstr /C:"client with the same version number as the upgrade package or a newer version is already installed" %INFO_TMP_FILE% >nul 2>nul && (
            @REM 系统不支持dataturbo，跳过安装
            call :LogAndPrint "Dataturbo client with the same version number as the upgrade package or a newer version is already installed, will skip upgrade."
            call "%AGENT_NEWPKG_BIN_PATH%\agent_start.bat" dataturbo
            if NOT !errorlevel! EQU 0 (
                call :LogAndPrint "Service dataturbo start failed, Upgrade dataturbo failed."
                exit /b 1
            )
            exit /b 0
        )
        call :LogAndPrint "Upgrade dataturbo failed."
        exit /b 1
    )
    if NOT exist "%INSTALLED_BY_AGENT%" (md "%INSTALLED_BY_AGENT%" && attrib +h "%INSTALLED_BY_AGENT%")
    call "%AGENT_NEWPKG_BIN_PATH%\agent_start.bat" dataturbo
    if NOT !errorlevel! EQU 0 (
        call :LogAndPrint "Service dataturbo start failed, Upgrade dataturbo failed."
        exit /b 1
    )
    call :LogAndPrint "Upgrade dataturbo successfully."
    exit /b 0
goto :EOF

:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOG_FILE%"
    exit /b 0

:LogAndPrint
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOG_FILE%"
    echo %~1
    exit /b 0

endlocal