rem
rem This file is a part of the open-eBackup project.
rem This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
rem If a copy of the MPL was not distributed with this file, You can obtain one at
rem http://mozilla.org/MPL/2.0/.
rem
rem Copyright (c) [2024] Huawei Technologies Co.,Ltd.
rem
rem THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
rem EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
rem MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
rem
@echo off
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set BASE_PATH=%CurBatPath%..
set INSTALL_LOG=%CurBatPath%install.log

call :main
endlocal
goto :eof

:create_path
    ::create param file path
    if not exist %BASE_PATH%\tmp mkdir %BASE_PATH%\tmp
    ::create result file path
    if not exist %BASE_PATH%\stmp mkdir %BASE_PATH%\stmp
goto :eof

:change_start_bat
    xcopy /y %BASE_PATH%\install\start.bat %BASE_PATH%\start.bat >> %INSTALL_LOG%
    del %BASE_PATH%\install\start.bat
goto :eof

:change_plugin_name
    if exist %BASE_PATH%\bin\AgentPlugin.exe ren %BASE_PATH%\bin\AgentPlugin.exe VirtualizationPlugin.exe
goto :eof

:main
    call :create_path

    call :change_start_bat

    call :change_plugin_name

goto :eof