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
setlocal EnableDelayedExpansion

set CurBatPath=%~dp0

call %CurBatPath%\install\install.bat
if %errorlevel% NEQ 0 (
    echo Failed to execute the DBPlugin install script!
    exit /b 1
)

endlocal

exit /b 0