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
rem @echo off
set CurBatPath=%~dp0
set PluginRootPath=%CurBatPath%..\..\..\
set ModuleRootPath=%PluginRootPath%AppPlugins_NAS\Module\
set Ext_Pkg=%CurBatPath%..\..\..\Ext_pkg
if "%1" EQU "" (
   set BRANCH=master_OceanProtect_DataBackup_1.6.0_smoke
) else (
   set BRANCH=%1
)

call :main
goto :eof

:clean
    rd /s /q  %PluginRootPath%Module
    rd /s /q  %PluginRootPath%framework
goto :eof

:download_module
    git clone -b %BRANCH% ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git %PluginRootPath%AppPlugins_NAS\Module
:download_framework
    git clone -b %BRANCH% ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectagent/AppPlugins_NAS.git %PluginRootPath%AppPlugins_NAS
goto :eof
:download_third_zip
    IF exist %Ext_Pkg%  rd /S /Q %Ext_Pkg%
    IF not exist %Ext_Pkg% mkdir %Ext_Pkg%

    ::artget.exe pull -d %CurBatPath%\opensrc_platofrm_dependencies.xml -p "{'COMPOENT_VERSION':'%COMPOENT_VERSION%','PRODUCT':'dorado', 'THIRD_BRANCH':'%BRANCH%','OS_TYPE':'%OS_TYPE%','SYSTEM_NAME':'%SYSTEM_NAME%'}" -ap %Ext_Pkg% -user %cmc_user% -pwd %cmc_pwd%

    ::7z.exe x -y "%Ext_Pkg%\OceanProtect_X8000_1.2.1RC1_Opensrc_3rd_SDK_Windows2012_x64.zip" -o%Ext_Pkg% >nul
    artget.exe pull -d %ModuleRootPath%build\LCRP\conf\code_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'open_src','COMPONENT_TYPE':'windows','ARCH':'open_src.zip'}" -ap %Ext_Pkg% -user %cmc_user% -pwd %cmc_pwd%

    7z.exe x -y "%Ext_Pkg%\open_src.zip" -o%Ext_Pkg% >nul
goto :eof
:main
    call :clean
    call :download_framework
    call :download_module
    call :download_third_zip
goto :eof
