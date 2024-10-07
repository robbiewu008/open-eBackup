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
set PLUGIN_JSON=%CurBatPath%..\conf\plugin_attribute_1.0.0.json
set BIN_PATH=%CurBatPath%..\bin
call :get_plugin_name

set LOG_FILE_PREFIX=%CurBatPath%..\..\..\ProtectClient-E\log\Plugins\%PluginName%
set LOG_FILE=%LOG_FILE_PREFIX%\install.log

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
    call :change_start_bat
    call :log_echo "INFO" "%PluginName% install success"
    endlocal
    exit /b 0

:create_dir
    if not exist %LOG_FILE_PREFIX% (
        md %LOG_FILE_PREFIX%
    )
    goto :eof

:change_start_bat
    xcopy /y %CurBatPath%start.bat %CurBatPath%..\start.bat > nul
    del %CurBatPath%start.bat
    @REM 区分不同插件的二进制
    move %BIN_PATH%\AgentPlugin.exe %BIN_PATH%\FilePlugin.exe > nul
    goto :eof

:get_plugin_name
    for /f tokens^=4^ delims^=^" %%a in ('type %PLUGIN_JSON% ^| findstr name') do (
        set PluginName=%%a
    )
    goto :eof

