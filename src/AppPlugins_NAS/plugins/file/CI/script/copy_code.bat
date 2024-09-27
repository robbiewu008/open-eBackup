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

@REM rem Copy file code
xcopy /y /e %PLUGIN_PATH%\framework\  %CopyPath%\framework\
xcopy /y /e %PLUGIN_PATH%\plugins\  %CopyPath%\plugins\
xcopy /y /e %PLUGIN_PATH%\FS_Scanner\  %CopyPath%\FS_Scanner\
xcopy /y /e %PLUGIN_PATH%\FS_Backup\  %CopyPath%\FS_Backup\
xcopy /y /e %PLUGIN_PATH%\Module\  %CopyPath%\Module\
echo d | xcopy /y /e %PLUGIN_PATH%\framework\  %CopyPath%\framework\
echo d | xcopy /y /e %PLUGIN_PATH%\plugins\  %CopyPath%\plugins\
echo d | xcopy /y /e %PLUGIN_PATH%\FS_Scanner\  %CopyPath%\FS_Scanner\
echo d | xcopy /y /e %PLUGIN_PATH%\FS_Backup\  %CopyPath%\FS_Backup\
echo d | xcopy /y /e %PLUGIN_PATH%\Module\  %CopyPath%\Module\
