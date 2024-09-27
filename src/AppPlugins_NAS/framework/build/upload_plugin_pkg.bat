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
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set PluginRootPath=%CurBatPath%..\..
echo PluginRootPath is %PluginRootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set UPLOAD_BRANCH=%1
)

artget.exe push -d %CurBatPath%LCRP\conf\pkg_into_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%UPLOAD_BRANCH%','COMPONENT_TYPE':'Plugins','ARCH':'Windows'}" -ap %PluginRootPath%\output_pkg\GeneralDBPlugin.zip -user %cmc_user% -pwd %cmc_pwd%
goto :eof
endlocal

goto :eof
     echo "Finish"
     Exit