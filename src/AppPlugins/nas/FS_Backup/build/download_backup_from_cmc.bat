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
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set RootPath=%CurBatPath%..
echo RootPath is %RootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set DOWNLOAD_BRANCH=%1
)
cd %RootPath%
if exist %RootPath%\libs  rd /S /Q %RootPath%\libs 
if not exist %RootPath%\libs  mkdir %RootPath%\libs 
artget.exe pull -d %CurBatPath%LCRP\conf\module_pkg_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%DOWNLOAD_BRANCH%','COMPONENT_TYPE':'FS_BACKUP','ARCH':'Windows/Backup.zip'}" -ap %RootPath%\libs -user %cmc_user% -pwd %cmc_pwd%
7z.exe e -y libs/Backup.zip -olibs/
del /a /f /s libs\Backup.zip
rd /s /q libs\Backup

goto :eof
endlocal



goto :eof
     echo "Finish"
     Exit