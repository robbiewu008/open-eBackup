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

rem @echo on
setlocal EnableDelayedExpansion
set Drive=%~d0
set CurBatPath=%~dp0
set CopyPath=%~1
echo CurBatPath is %CurBatPath%
set PLUGIN_PATH=%CurBatPath%..\..\..\..

call ..\..\build\build_file.bat
if %errorlevel% NEQ 0 (
    echo Failed to execute the build_file script! >> %PACK_LOG%
    exit 1
)

@REM rem Copy file bin
if not exist %CopyPath%\Windows_Compile mkdir %CopyPath%\Windows_Compile

xcopy /y /e %PLUGIN_PATH%\Windows_Compile\plugins\  %CopyPath%\Windows_Compile\plugins\
xcopy /y /e %PLUGIN_PATH%\Windows_Compile\framework\  %CopyPath%\Windows_Compile\framework\
xcopy /y /e %PLUGIN_PATH%\Windows_Compile\FS_Scanner\  %CopyPath%\Windows_Compile\FS_Scanner\
xcopy /y /e %PLUGIN_PATH%\Windows_Compile\FS_Backup\  %CopyPath%\Windows_Compile\FS_Backup\
xcopy /y /e %PLUGIN_PATH%\Windows_Compile\Module\  %CopyPath%\Windows_Compile\Module\
xcopy /y /e %PLUGIN_PATH%\Windows_Compile\vsprj\  %CopyPath%\Windows_Compile\vsprj\
echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\plugins\  %CopyPath%\Windows_Compile\plugins\
echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\framework\  %CopyPath%\Windows_Compile\framework\
echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\FS_Scanner\  %CopyPath%\Windows_Compile\FS_Scanner\
echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\FS_Backup\  %CopyPath%\Windows_Compile\FS_Backup\
echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\Module\  %CopyPath%\Windows_Compile\Module\
echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\vsprj\  %CopyPath%\Windows_Compile\vsprj\
