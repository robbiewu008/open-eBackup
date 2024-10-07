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
set FrameworkRootPath=%CurBatPath%..
echo PluginRootPath is %PluginRootPath%


cd %CurBatPath%
if "%SDK_BRANCH%" == "" (
    set SDK_BRANCH=%branch%
)
if "%SDK_BRANCH%" == "" (
    set SDK_BRANCH=develop_backup_software_1.6.0RC1
)

if exist %PluginRootPath%\EXT_Pkg  rd /S /Q %PluginRootPath%\EXT_Pkg
if not exist %PluginRootPath%\EXT_Pkg  mkdir %PluginRootPath%\EXT_Pkg

if exist %FrameworkRootPath%\vc_redist  rd /S /Q %FrameworkRootPath%\vc_redist
if not exist %FrameworkRootPath%\vc_redist  mkdir %FrameworkRootPath%\vc_redist

artget.exe pull -d %CurBatPath%LCRP\conf\nas_3rd_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%SDK_BRANCH%','COMPONENT_TYPE':'dependency','ARCH':'Windows/plugin_sdk.zip'}" -ap %PluginRootPath%\EXT_Pkg -user %cmc_user% -pwd %cmc_pwd%
artget.exe pull -d %CurBatPath%LCRP\conf\nas_3rd_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'develop','COMPONENT_TYPE':'vc_redist','ARCH':''}" -ap %FrameworkRootPath%\vc_redist -user %cmc_user% -pwd %cmc_pwd%

7z.exe x -y "%PluginRootPath%\EXT_Pkg\plugin_sdk.zip" -o%PluginRootPath%\EXT_Pkg


set VsprjPath=%PluginRootPath%\Windows_Compile\vsprj
echo VsprjPath is %VsprjPath%

COPY /Y  %PluginRootPath%\EXT_Pkg\lib\agentsdk.lib               %VsprjPath%\bin\bin\

COPY /Y  %PluginRootPath%\EXT_Pkg\lib\agentsdk.dll              %VsprjPath%\bin\bin\

if not exist %PluginRootPath%\framework\dep\agent_sdk  mkdir %PluginRootPath%\framework\dep\agent_sdk
if not exist %PluginRootPath%\framework\dep\agent_sdk\include  mkdir %PluginRootPath%\framework\dep\agent_sdk\include
xcopy /y /e %PluginRootPath%\EXT_Pkg\include   %PluginRootPath%\framework\dep\agent_sdk\include


endlocal

goto :eof
     echo "Finish"
     Exit