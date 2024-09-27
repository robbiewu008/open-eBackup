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

rem @echo off
set CurBatPath=%~dp0
set PluginBranch=%CurBatPath%..\..\..\
if "%1" EQU "" (
   set BRANCH=master_OceanProtect_DataBackup
) else (
   set BRANCH=%1
)

call :main
goto :eof

:clean
    rd /s /q  %PluginBranch%Module
    rd /s /q  %PluginBranch%FS_Scanner
    rd /s /q  %PluginBranch%FS_Backup
goto :eof

:download_dependency
    git clone -b %BRANCH% ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git %PluginBranch%\Module
    git clone -b %BRANCH% ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/FS_Scanner.git %PluginBranch%\FS_Scanner
    git clone -b %BRANCH% ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/FS_Backup.git %PluginBranch%\FS_Backup

goto :eof

:main
    call :clean
    call :download_dependency
goto :eof