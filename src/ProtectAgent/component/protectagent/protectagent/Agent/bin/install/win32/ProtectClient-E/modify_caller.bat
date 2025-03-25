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
set LOGFILE_PATH=%AGENT_ROOT_PATH%\log\modify_pre.log
set LOG_ERR_FILE=%AGENT_ROOT_PATH%\tmp\errormsg.log

set AGENT_MANAGER_PATH=%AGENT_ROOT_PATH%\..
set AGENT_PKG_INSTALL_PATH=%AGENT_MANAGER_PATH%\..
set AGENT_MODIFY_PACKAGE_PATH=%AGENT_PKG_INSTALL_PATH%\PushModify

set MODIFY_PACKAGE_UNZIP_NAME=
set MODIFY_SCRIPT_PATH=

set PRODUCT_NAME=DataProtect

set ERR_MODIFY_FAIL_VERIFICATE_PARAMETER=1577209881
set ERR_MODIFY_SCRIPT_NOT_FOUND=1577210141
set ERR_MODIFY_SCRIPT_NOT_FOUND_RETCODE=81

rem #################### Main process ##########################
call :Log "Modify caller begin."

call :Log "Start check the Modify Script."
call :CheckModifyScript
if NOT %errorlevel% EQU 0 (
    call :LogError "Check pac unique unsuccessfully." %ERR_MODIFY_SCRIPT_NOT_FOUND%
    exit %ERR_MODIFY_SCRIPT_NOT_FOUND_RETCODE%
)

call :Log "Begin to call modify script, and exec mode is /push."
rem Move modify_pre.log and errormsg.log to the temporary directory.
copy /y %LOGFILE_PATH% %AGENT_MODIFY_PACKAGE_PATH% >nul
start /high /wait cmd /c %MODIFY_SCRIPT_PATH% /push
if NOT %errorlevel% EQU 0(
    set RES=%errorlevel%
    exit %RES%
)
endlocal
exit 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
goto :EOF

:LogError
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_modify_prepare_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
goto :EOF

:CheckModifyScript
    for /f "delims=: tokens=1,2" %%a in ('dir /b "%AGENT_MODIFY_PACKAGE_PATH%" ^| findstr %PRODUCT_NAME% ^| findstr /v .zip ^| findstr /n .*') do (
        set MODIFY_PACKAGE_UNZIP_NAME=%%b
    )
    set MODIFY_SCRIPT_PATH=%AGENT_MODIFY_PACKAGE_PATH%\%MODIFY_PACKAGE_UNZIP_NAME%\modify.bat
    if NOT exist "%MODIFY_SCRIPT_PATH%" (
        call :Log "The modify script [package %MODIFY_SCRIPT_PATH%] not exists."
        exit /b 1
    )
    
    exit /b 0
goto :EOF