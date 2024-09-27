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

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set PLUGIN_FRAMEWORK_PATH="%CurBatPath%..\..\"
set BUILD_LOG="%CurBatPath%"Windows_Compile\build.log
set VsprjPath=%PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj

rem 获得系统的日期和时间
set DAY=%date:~8,2%
set MOUNT=%date:~5,2%
set YEAR=%date:~0,4%
set TIME=%YEAR%-%MOUNT%-%DAY%

rem===================================================
echo 1 is %1
if "%1" EQU "Clean" (GOTO Clean)

if not "%errorlevel%" == "0" (exit 1)

SET PACKAGE_FILE_DIR=%PLUGIN_FRAMEWORK_PATH%framework\output_pkg
SET LIB_FILE_DIR=%PLUGIN_FRAMEWORK_PATH%framework\lib
SET LIB_VC_REDIST=%PLUGIN_FRAMEWORK_PATH%framework\vc_redist

IF not EXIST %PACKAGE_FILE_DIR%\bin   MKDIR  %PACKAGE_FILE_DIR%\bin
IF not EXIST %PACKAGE_FILE_DIR%\conf  MKDIR %PACKAGE_FILE_DIR%\conf
IF not EXIST %PACKAGE_FILE_DIR%\stmp  MKDIR %PACKAGE_FILE_DIR%\stmp
IF not EXIST %PACKAGE_FILE_DIR%\tmp  MKDIR %PACKAGE_FILE_DIR%\tmp

rem==================2. copy bin file begin=======================
IF NOT EXIST %LIB_FILE_DIR%\AgentPlugin.exe   call :FileNotFound AgentPlugin.exe
COPY /Y %LIB_FILE_DIR%\*.dll                    %PACKAGE_FILE_DIR%\bin
COPY /Y %PLUGIN_FRAMEWORK_PATH%\plugins\file\lib\*.dll    %PACKAGE_FILE_DIR%\bin
COPY /Y %LIB_VC_REDIST%\*.dll    %PACKAGE_FILE_DIR%\bin

rem COPY /Y %LIB_FILE_DIR%\*.lib                    %PACKAGE_FILE_DIR%\bin
COPY /Y %LIB_FILE_DIR%\AgentPlugin.exe          %PACKAGE_FILE_DIR%\bin

IF NOT EXIST %CurBatPath%..\conf\app_lib.json                       call :FileNotFound app_lib.json
COPY /Y %CurBatPath%..\conf\app_lib.json                %PACKAGE_FILE_DIR%\conf\app_lib.json

IF NOT EXIST %CurBatPath%\install\start.bat                       call :FileNotFound start.bat 
COPY /Y %CurBatPath%\install\start.bat                %PACKAGE_FILE_DIR%\start.bat

IF NOT EXIST %CurBatPath%\install\stop.bat                       call :FileNotFound stop.bat 
COPY /Y %CurBatPath%\install\stop.bat                %PACKAGE_FILE_DIR%\stop.bat

IF NOT EXIST %CurBatPath%\install\install.bat                       call :FileNotFound install.bat 
COPY /Y %CurBatPath%\install\install.bat                %PACKAGE_FILE_DIR%\install.bat

IF NOT EXIST %CurBatPath%\install\uninstall.bat                       call :FileNotFound uninstall.bat 
COPY /Y %CurBatPath%\install\uninstall.bat                %PACKAGE_FILE_DIR%\uninstall.bat

echo  %DATE% %TIME% %1 copy nginx files >> %BUILD_LOG%

rem==================3. zip plugin framework begin========================
cd %PACKAGE_FILE_DIR%\conf\
IF NOT EXIST %PACKAGE_FILE_DIR%\conf\plugin_attribute_1.0.0.json  call :FileNotFound plugin_attribute_1.0.0.json
COPY /Y %PACKAGE_FILE_DIR%\conf\plugin_attribute_1.0.0.json                %PACKAGE_FILE_DIR%\plugin_attribute_1.0.0.json
for /f tokens^=^4^ delims^=^" %%i in ('findstr name plugin_attribute_1.0.0.json') do (
    set PluginNameStr=%%i
)
if not defined PluginNameStr  set  PluginNameStr=GeneralDBPlugin
echo %PluginNameStr%
cd %CurBatPath%
7z.exe  a "%PACKAGE_FILE_DIR%\%PluginNameStr%.zip" "%PACKAGE_FILE_DIR%\*"

echo  %DATE% %TIME% %1 Finished to zip Plugin >> %BUILD_LOG%


echo Release Success. 
rem exit 0


:FileNotFound
echo  %DATE% %TIME% %1 Not Found >> %BUILD_LOG%
goto Exit 

:Clean
rem call %PLUGIN_FRAMEWORK_PATH%build\ms\BCM\agent_make.bat Clean64Openssl
rem goto Exit

:Exit
rem exit 1
            