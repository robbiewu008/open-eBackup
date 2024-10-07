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

rem @echo offs
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set BackupRootPath=%CurBatPath%..
echo BackupRootPath is %BackupRootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set UPLOAD_BRANCH=%1
)
call :compress_backup_pkg
artget.exe push -d %CurBatPath%LCRP\conf\pkg_into_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%UPLOAD_BRANCH%','COMPONENT_TYPE':'FS_BACKUP','ARCH':'Windows/'}" -ap %BackupRootPath%\FS_BACKUP.zip -user %cmc_user% -pwd %cmc_pwd%
@REM del /f /q Backup.zip
goto :eof
endlocal

:compress_backup_pkg:
   cd %BackupRootPath%
   7z.exe a FS_BACKUP.zip %BackupRootPath%\libs\FS_Backup.dll
   goto :eof

goto :eof
     echo "Finish"
     Exit