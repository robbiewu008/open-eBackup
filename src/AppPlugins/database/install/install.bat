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
set INSTALL_LOG=%CurBatPath%..\..\..\ProtectClient-E\log\Plugins\GeneralDBPlugin\install.log
set BIN_PATH=%CurBatPath%..\bin

call :main
endlocal
goto :eof

:create_path
    ::create param file path
    if not exist %BASE_PATH%\tmp mkdir %BASE_PATH%\tmp
    ::create result file path
    if not exist %BASE_PATH%\stmp mkdir %BASE_PATH%\stmp
goto :eof

:get_os
    @REM Windows 2008不支持安装python3.10.2
    ver | find "6.1" > nul && set TheOS=Windows 2008
    if "%TheOS%"=="Windows 2008" (
        call :not_support_python
    ) else (
        call :change_start_bat
        call :create_python_env
    )
goto :eof

:change_start_bat
    xcopy /y %BASE_PATH%\install\start.bat %BASE_PATH%\start.bat >> %INSTALL_LOG%
    move %BIN_PATH%\AgentPlugin.exe %BIN_PATH%\GeneralDBPlugin.exe > nul
    del %BASE_PATH%\install\start.bat
goto :eof

:create_python_env
    set python3_file=python3.pluginFrame.win32.zip
    set zipTool=%CurBatPath%..\..\..\ProtectClient-E\bin\7z.exe
    %zipTool% x -y "%BASE_PATH%\install\%python3_file%" -o%BASE_PATH%\install >> %INSTALL_LOG%
    if %errorlevel% NEQ 0 (
        echo Failed to unzip %python3_file% ! >> %INSTALL_LOG%
        exit /b 1
    )
    del %BASE_PATH%\install\%python3_file%
    call %BASE_PATH%\install\python_env.bat
    if %errorlevel% NEQ 0 (
        echo Failed to execute the python_env script ! >> %INSTALL_LOG%
        exit /b 1
    )
goto :eof

:not_support_python
    echo The current system does not support install python 3.10.2 ! >> %INSTALL_LOG%
goto :eof

:main
    call :create_path

    call :get_os
goto :eof