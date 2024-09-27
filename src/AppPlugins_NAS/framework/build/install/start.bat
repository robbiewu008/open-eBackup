@REM
@REM This file is a part of the open-eBackup project.
@REM This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
@REM If a copy of the MPL was not distributed with this file, You can obtain one at
@REM http://mozilla.org/MPL/2.0/.
@REM
@REM Copyright (c) [2024] Huawei Technologies Co.,Ltd.
@REM
@REM THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
@REM EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
@REM MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
@REM

@echo off
rem ########################################################################
rem # NasPlugin start.bat
rem ########################################################################
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo %CurBatPath%
set PLUGIN_JSON=%CurBatPath%conf\plugin_attribute_1.0.0.json
call :get_plugin_name

set LOG_FILE_PREFIX=%CurBatPath%..\..\ProtectClient-E\log\Plugins\%PluginName%
set LOG_FILE=%LOG_FILE_PREFIX%\start.log

call :main %*
goto :eof

:log_echo
    setlocal
    set logType=%~1
    set message=%~2
    echo [%date% - %time%] [%logType%] [%message%] [%~nx0] >> %LOG_FILE%
    endlocal
    goto :eof

:main
    call :create_dir
    call :check_paramters %*
    if ret == 1 (
        exit 1
    )
    IF NOT DEFINED DATA_BACKUP_AGENT_HOME (
        @rem 添加环境变量
        SETX DATA_BACKUP_AGENT_HOME C:
        @rem 使环境变量在当前进程可见
        SET DATA_BACKUP_AGENT_HOME=C:
    )
    start %DATA_BACKUP_AGENT_HOME%\DataBackup\ProtectClient\Plugins\%PluginName%\bin\AgentPlugin.exe %LOG_PATH% %PORT_BEGIN% %PORT_END% %AGENT_IP% %AGENT_PORT%
    call :log_echo "INFO" "%PluginName% start success"
    endlocal
    exit 0

:create_dir
    if not exist %LOG_FILE_PREFIX% (
        md %LOG_FILE_PREFIX%
    )
    goto :eof

:check_paramters
    set ret=0
    set count=0
    for %%x in (%*) do set /a count+=1
    if not %count% == 5 (
        call :log_echo "ERR" "the script is asked to be five parameters"
        set ret=1
        goto :eof
    )
    set LOG_PATH=%~1
    set PORT_BEGIN=%~2
    set PORT_END=%~3
    set AGENT_IP=%4
    set AGENT_PORT=%5
    echo %PORT_BEGIN% | findstr /r "^[1-9][0-9][0-9][0-9][0-9]*"
    if not %errorlevel% == 0 (
        call :log_echo "ERR" "the port begin is below 1000"
        goto :eof
        exit 1
    )
    echo %PORT_END% | findstr /r "^[1-9][0-9][0-9][0-9][0-9]*"
    if not %errorlevel% == 0 (
        call :log_echo "ERR" "the port end is below 1000"
        exit 1
        goto :eof

    )
    if %PORT_END% LSS %PORT_BEGIN% (
        call :log_echo "ERR" "port of begin below port of end"
        exit 1
        goto :eof
    )
    echo @%AGENT_IP%@ | findstr /r  "@[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*@"
    if not %errorlevel% == 0 (
        call :log_echo "ERR" "ip address has a wrong format"
        exit 1
        goto :eof
    )
    
    goto :eof

:get_plugin_name
    for /f tokens^=4^ delims^=^" %%a in ('type %PLUGIN_JSON% ^| findstr name') do (
        set PluginName=%%a
    )
    goto :eof