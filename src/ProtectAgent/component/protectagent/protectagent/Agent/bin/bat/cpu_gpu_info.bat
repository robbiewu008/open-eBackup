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

set AGENT_ROOT_PATH=%1
set RANDOM_ID=%2
cd /d %~dp0

set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\\bin\\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\\log\\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\\tmp\\
set NEED_LOG=1
set LOGFILE_PATH=%AGENT_LOG_PATH%cpu_gpu_info.log
set INPUT_TMP_FILE=%AGENT_TMP_PATH%input_tmp%RANDOM_ID%
set RESULT_FILE=%AGENT_TMP_PATH%result_tmp%RANDOM_ID%

set ERR_CODE=1
set ERR_SCRIPT_EXEC_FAILED=5
set ERR_SCRIPT_NOT_EXIST=255
set ERR_RESULT_FILE_NOT_EXIST=6
set ERR_INPUT_TEMP_NOT_EXSIT=7
set ERR_FILE_IS_EMPTY=8
set ERR_REMOVE_FAILED=130

rem Parse the InputInfo
call :log "Begin to parse the InputInfo."

if not exist %INPUT_TMP_FILE% (
    set ERR_CODE=%ERR_INPUT_TEMP_NOT_EXSIT%
    call :log "input_tmp%RANDOM_ID% file is not exist, exit code is %ERR_CODE%."
    goto error
)

for /f "delims=" %%i in ('type %INPUT_TMP_FILE%') do (
    if not "%%i" == "" (
        set INPUTINFO="%%i"
    )
)

if exist %INPUT_TMP_FILE% (del /f /q %INPUT_TMP_FILE%)
call :log "INPUTINFO=!INPUTINFO!"

rem Get powershell path
call :log "Begin to get powershell path."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1\PowerShellEngine" /v "ApplicationBase"') do set "POWERSHELL_PATH=%%k"
if "!POWERSHELL_PATH!"=="" (
    call :log "Get powershell path faild."
    set ERR_CODE=%ERR_FILE_IS_EMPTY%
    goto error
)
call :log "Get powershell path succ, powershell path %POWERSHELL_PATH%."

if %INPUTINFO%=="cpu" (
    rem Excute powershell script to get cpu info
    if not exist %AGENT_BIN_PATH%cpu_gpu_info.ps1 (
        set ERR_CODE=%ERR_SCRIPT_NOT_EXIST%
        call :log "Script cpu_gpu_info.ps1 is not exist, exit code %ERR_CODE%"
        goto error
    )

    call :log "Begin to execute cpu_gpu_info.ps1 powershell, result file result_tmp%RANDOM_ID%."
    "%POWERSHELL_PATH%\powershell.exe" -command ".\cpu_gpu_info.ps1 '%INPUTINFO%' '%RESULT_FILE%'"; exit $LASTEXITCODE

    if errorlevel 1 (
    	if exist %RESULT_FILE% (
            for /f "delims=" %%i in ('type %RESULT_FILE%') do (
                if not "%%i" == "" (
                    set ERR_CODE=%%i
                )
            )
            del /f /q %RESULT_FILE%
            call :log "Execute powershell[cpu_gpu_info.ps1] for getting cpu info failed, exit code !ERR_CODE!."
            goto error
        )
        call :log "Execute powershell[cpu_gpu_info.ps1] for getting cpu info failed, errorlevel is not 0."
        set ERR_CODE=%ERR_SCRIPT_EXEC_FAILED%
        goto error
    ) else (
        if not exist %RESULT_FILE% (
            set ERR_CODE=%ERR_RESULT_FILE_NOT_EXIST%
            call :log "Execute powershell[cpu_gpu_info.ps1] for getting cpu info failed, result file is not exist, exit code !ERR_CODE!"
            goto error
        )
        call :log "Execute powershell[cpu_gpu_info.ps1] for getting cpu info succ. errorlevel is [!errorlevel!]"
        goto end
    )
) else if %INPUTINFO%=="gpu" (
    rem Excute powershell script to get gpu info
    if not exist %AGENT_BIN_PATH%cpu_gpu_info.ps1 (
        set ERR_CODE=%ERR_SCRIPT_NOT_EXIST%
        call :log "Script cpu_gpu_info.ps1 is not exist, exit code !ERR_CODE!"
        goto error
    )

    call :log "Begin to execute powershell."
    "%POWERSHELL_PATH%\powershell.exe" -command ".\cpu_gpu_info.ps1 '%INPUTINFO%' '%RESULT_FILE%'"; exit $LASTEXITCODE

    if errorlevel 1 (
        if exist %RESULT_FILE% (

            for /f "delims=" %%i in ('type %RESULT_FILE%') do (
                if not "%%i" == "" (
                    set ERR_CODE=%%i
                )
            )
            del /f /q %RESULT_FILE%
            call :log "Execute powershell[cpu_gpu_info.ps1] for getting gpu info failed, exit code !ERR_CODE!."
            goto error
        )

        set ERR_CODE=%ERR_SCRIPT_EXEC_FAILED%
        call :log "Execute powershell[cpu_gpu_info.ps1] for getting gpu info failed, exit code !ERR_CODE!."
        goto error
    ) else (
        call :log "Execute powershell[cpu_gpu_info.ps1] for getting gpu info succ. errorlevel is [!errorlevel!]"
        goto end
    )
) else (
        call :log "Execute powershell[cpu_gpu_info.ps1] failed with a erorr parameter[!INPUTINFO!]."
        goto end
)

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILE_PATH%
    )
    call %AGENT_BIN_PATH%\agent_func.bat %LOGFILE_PATH%
    goto :EOF

:error
    call :log "Get cpu or gpu info failed."
    exit /b %ERR_CODE%

:end
    endlocal

