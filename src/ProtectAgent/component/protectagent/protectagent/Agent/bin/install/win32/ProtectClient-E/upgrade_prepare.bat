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

set CURRENT_PATH=%~dp0

set AGENT_BIN_PATH=%CURRENT_PATH%
set AGENT_ROOT_PATH=%AGENT_BIN_PATH%\..
set AGENT_MANAGER_PATH=%AGENT_ROOT_PATH%\..
set AGENT_PKG_INSTALL_PATH=%AGENT_MANAGER_PATH%\..

set LOG_FILE=%AGENT_ROOT_PATH%\log\upgrade_pre.log
set LOG_ERR_FILE=%AGENT_ROOT_PATH%\tmp\errormsg.log
set UPGRADE_INFO_FILE=%AGENT_ROOT_PATH%\tmp\tmpUpgradeInfo
set AGENTCOF_XML_FILE=%AGENT_ROOT_PATH%\conf\agent_cfg.xml

set AGENT_UPGRADE_PACKAGE_PATH=%AGENT_PKG_INSTALL_PATH%\PushUpgrade\
set PUBLIC_KEY_FILE=%AGENT_ROOT_PATH%\upgrade\upgrade_public_key.pem
set SIGNATURE_FILE=%AGENT_ROOT_PATH%\upgrade\upgrade_signature.sign
set SHA256_VALUE_FILE=%AGENT_ROOT_PATH%\upgrade\sha256value

set UPGRADE_PACKAGE_NAME=
set PRODUCT_NAME=DataProtect
set /p SHA256_VALUE_ORIGINAL=
set UPGRADE_PACKAGE_SHA256_VALUE=
set SHA256_VALUE_FINAL=
set PACKAGE_COUNT= 0
set ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER=1577209881
set LOOP_COUNT=0

rem -------------error code----------------------
set /a ERR_PKG_NOT_COMPLETE=1677931366
set /a ERR_UNZIP_PKG=1677931367
rem -------------error code----------------------

rem #################### Main process ##########################
call :Log "Upgrade prepare begin."
move %CURRENT_PATH%\..\tmp\DataProtect_client.zip %AGENT_UPGRADE_PACKAGE_PATH%

call :Log "Start check package unique."
call :CheckPacUnique
if NOT %errorlevel% EQU 0 (
    call :LogError "Check pac unique unsuccessfully." %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
    exit %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
)

call :Log "Start check package complete."
call :CheckShaValue
if NOT %errorlevel% EQU 0 (
    call :LogError "Check sha256 value unsuccessfully." %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
    exit %ERR_PKG_NOT_COMPLETE%
)

call :VerifySignature
if NOT %errorlevel% EQU 0 (
    call :LogError "Verify upgrade file signature unsuccessfully." %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
    exit %ERR_PKG_NOT_COMPLETE%
)

call :Log "Start decompress the upgrade package."
call :UnzipUpgradeFile
if NOT %errorlevel% EQU 0 (
    call :LogError "Unzip upgrade file unsuccessfully." %ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER%
    exit %ERR_UNZIP_PKG%
)

call :Log "Upgrade prepare successfully."
endlocal
exit 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOG_FILE%"
goto :EOF

:LogError
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_update_prepare_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
goto :EOF

:CheckPacUnique
    for /f "delims=: tokens=1,2" %%a in ('dir /b "%AGENT_UPGRADE_PACKAGE_PATH%" ^| findstr %PRODUCT_NAME% ^| findstr /n .*') do (
        set /a PACKAGE_COUNT+=1
        set UPGRADE_PACKAGE_NAME=%%b
    )
    if NOT %PACKAGE_COUNT% EQU 1 (
        call :Log "Check package uniqueness failed, UPGRADE_PACKAGE_COUNT=[%PACKAGE_COUNT%]."
        exit /b 1
    ) else (
        call :Log "Check package uniqueness failed."
        exit /b 0
    )
goto :EOF

:CheckShaValue
    if NOT exist "%UPGRADE_INFO_FILE%" (
        call :Log "Not find the upgrade infomation file [tmpUpgradeInfo]."
        exit /b 1
    )

    rem Generate SHA256 value    
    for /f "delims=: tokens=1*" %%a in ('certutil -hashfile "%AGENT_UPGRADE_PACKAGE_PATH%%UPGRADE_PACKAGE_NAME%" SHA256 ^| findstr /n .*') do (
        if %%a EQU 2 (
            set UPGRADE_PACKAGE_SHA256_VALUE=%%b
        )
    )
    for %%a in (%UPGRADE_PACKAGE_SHA256_VALUE%) do (
        set SHA256_VALUE_FINAL=!SHA256_VALUE_FINAL!%%a
    )

    if NOT "%SHA256_VALUE_ORIGINAL%" EQU "%SHA256_VALUE_FINAL%" (
        call :Log "Check sha256 failed, original sha256 value [%SHA256_VALUE_ORIGINAL%], current sha256 value [%SHA256_VALUE_FINAL%]."
        exit /b 1
    ) else (
        call :Log "Check sha256 successfully, original sha256 value [%SHA256_VALUE_ORIGINAL%], current sha256 value [%SHA256_VALUE_FINAL%]."
        exit /b 0
    )
goto :EOF

:UnzipUpgradeFile
    if NOT exist "%AGENT_BIN_PATH%\7z.exe" (
        call :Log "Can't find 7z.exe in %AGENT_BIN_PATH%\7z.exe."
        exit /b 1
    )

    "%AGENT_BIN_PATH%\7z.exe" x -tzip -y "%AGENT_UPGRADE_PACKAGE_PATH%%UPGRADE_PACKAGE_NAME%"  -o"%AGENT_UPGRADE_PACKAGE_PATH%" >nul
    exit /b 0
goto :EOF

:VerifySignature
    call :Log "Start verify upgrade pkg signature."
    set /p =%SHA256_VALUE_ORIGINAL%<nul >> %SHA256_VALUE_FILE%
    @REM openssl.exe dgst -sha256 -verify <公钥文件> -signature <签名文件> <sha256文件>
    %AGENT_BIN_PATH%\openssl.exe dgst -sha256 -verify %PUBLIC_KEY_FILE% -signature %SIGNATURE_FILE% %SHA256_VALUE_FILE% >> %LOG_FILE% 2>&1
    if NOT %errorlevel% EQU 0 (
        call :Log "Verify upgrade file signature failed."
        del /f /q  %SIGNATURE_FILE%
        del /f /q  %SHA256_VALUE_FILE%
        exit /b 1
    ) else (
        call :Log "Verify upgrade file signature successfully."
        del /f /q  %SIGNATURE_FILE%
        del /f /q  %SHA256_VALUE_FILE%
        exit /b 0
    )
goto :EOF