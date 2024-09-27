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

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set build_type=%1

set PLUGIN_FRAMEWORK_PATH=%CurBatPath%..\..\
echo PLUGIN_FRAMEWORK_PATH is %PLUGIN_FRAMEWORK_PATH%

if "%Module_Branch%" == "" (
    if "%branch%" == "" (
        set Module_Branch=debug_OceanProtect_DataBackup_1.6.0_openeBackup_v2
    ) else (
        set Module_Branch=%branch%
    )
)

rem create windows compile folder
cd "%CurBatPath%"
if exist %PLUGIN_FRAMEWORK_PATH%Windows_Compile  rd /S /Q "%PLUGIN_FRAMEWORK_PATH%Windows_Compile"
IF not EXIST %PLUGIN_FRAMEWORK_PATH%Windows_Compile   MKDIR  %PLUGIN_FRAMEWORK_PATH%Windows_Compile
IF not EXIST %PLUGIN_FRAMEWORK_PATH%Windows_Compile\framework   MKDIR  %PLUGIN_FRAMEWORK_PATH%Windows_Compile\framework
IF not EXIST %PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj   MKDIR  %PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj
IF not EXIST %PLUGIN_FRAMEWORK_PATH%Windows_Compile\Module   MKDIR  %PLUGIN_FRAMEWORK_PATH%Windows_Compile\Module
IF not EXIST %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\vsprj\bin   MD  %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\vsprj\bin\bin


rem Copy external pkg
cd "%CurBatPath%"
call download_dep_pkg.bat

rem copy the codes into windows compile folder
xcopy /y /e %PLUGIN_FRAMEWORK_PATH%framework  %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\framework >nul
xcopy /y /e %PLUGIN_FRAMEWORK_PATH%vsprj  %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\vsprj >nul
xcopy /y /e %PLUGIN_FRAMEWORK_PATH%\Module  %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module >nul

cd %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module

if "%build_type%" == "OPENSOURCE" (
    call %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\build\download_3rd_opensource.bat
) else (
    call %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\build\download_3rd.bat
)

xcopy /y /e %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\all_libs  %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\vsprj\bin\bin\

call %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\build\download_module_from_cmc.bat %Module_Branch%
echo d | xcopy /y /e %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\libs %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\vsprj\bin\bin

rem Genrate thrift cpp file
cd /d %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\framework\thrift_files
%PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\open_src\all_libs\thrift.exe -o . -r -gen cpp .\ApplicationProtectBaseDataType.thrift
%PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\open_src\all_libs\thrift.exe -o . -r -gen cpp  .\ApplicationProtectFramework.thrift
%PLUGIN_FRAMEWORK_PATH%\Windows_Compile\Module\open_src\all_libs\thrift.exe -o . -r -gen cpp  .\ApplicationProtectPlugin.thrift
del /q .\gen-cpp\*skeleton.cpp
move /y .\gen-cpp\*.cpp %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\framework\src\thrift_interface
IF not EXIST %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\framework\inc\thrift_interface MKDIR  %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\framework\inc\thrift_interface
move /y .\gen-cpp\*.h %PLUGIN_FRAMEWORK_PATH%\Windows_Compile\framework\inc\thrift_interface

cd "%PLUGIN_FRAMEWORK_PATH%Windows_Compile%"
git-bash.exe -c "cd Module/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"
git-bash.exe -c "cd framework/src;find -name *.cpp | xargs unix2dos;cd ../inc; find -name *.h | xargs unix2dos;"

set SOLUTION=%PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj\build\Plugin_Framework.sln
IF not EXIST %SOLUTION%  call :FileNotFound

echo SOLUTION is %SOLUTION%

set BUILD_LOG="%PLUGIN_FRAMEWORK_PATH%Windows_Compile\build.log"
echo BUILD_LOG is %BUILD_LOG%

set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

set VCVARS32_BAT="%VCTOOL_HOME%\VC\Auxiliary\Build\vcvars32.bat"
echo VCVARS32_BAT is %VCVARS32_BAT% >> %BUILD_LOG%

set VCVARSX86_AMD64_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvarsx86_amd64.bat
echo VCVARSX86_AMD64_BAT is %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%

set DEVEVN_FILE=%VCTOOL_HOME%\Common7\IDE\devenv.exe
echo DEVEVN_FILE is %DEVEVN_FILE% >> %BUILD_LOG%

echo include is %INCLUDE%

echo 1 is "%1"

if "%1" EQU "Compile32PluginFramework" (call :Compile32PluginFramework)
if "%1" EQU "Compile64PluginFramework" (call :Compile64PluginFramework)

call :Compile64PluginFramework
IF not exist %PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj\bin\bin\AgentPlugin.exe    CALL :FileNotFound

rem Copy lib into framework/lib
IF EXIST %PLUGIN_FRAMEWORK_PATH%\framework\lib  rd /S /Q "%PLUGIN_FRAMEWORK_PATH%\framework\lib" 
IF not EXIST %PLUGIN_FRAMEWORK_PATH%\framework\lib MKDIR  %PLUGIN_FRAMEWORK_PATH%\framework\lib
COPY /Y %PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj\bin\bin\*.dll      %PLUGIN_FRAMEWORK_PATH%\framework\lib
COPY /Y %PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj\bin\bin\*.lib      %PLUGIN_FRAMEWORK_PATH%\framework\lib
COPY /Y %PLUGIN_FRAMEWORK_PATH%Windows_Compile\vsprj\bin\bin\*.exe      %PLUGIN_FRAMEWORK_PATH%\framework\lib
SET PACKAGE_FILE_DIR=%PLUGIN_FRAMEWORK_PATH%framework\output_pkg
IF exist %PACKAGE_FILE_DIR%   rd /S /Q "%PACKAGE_FILE_DIR%"
IF not EXIST %PACKAGE_FILE_DIR%   MKDIR  "%PACKAGE_FILE_DIR%"

echo End of Compile
goto :eof


:Compile32PluginFramework
	rem begin to make openssl 32 lib file
	rem setting the enviroment with VC command 32
	set SOLUTION_CONFIG="Release|Win32"
	set ACTION=Rebuild
	call %VCVARS32_BAT%

	%Drive%
	cd %CurBatPath%

	echo Begin to clean compile >> %BUILD_LOG%
	echo %DEVEVN_FILE% "%SOLUTION%" /clean
	%DEVEVN_FILE% "%SOLUTION%" /clean

	echo Begin to rebuild the Agent >> %BUILD_LOG%
	echo %DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv

	%DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

	echo End to build x32 Agent
goto :eof

:Compile64PluginFramework
    %Drive%
    rem build src
    cd %CurBatPath%
    set SOLUTION_CONFIG="Release|x64"
    set ACTION=Rebuild
	call %VCVARSX86_AMD64_BAT%
    echo Begin to clean compile >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SOLUTION%" /clean

    %DEVEVN_FILE% %SOLUTION% /clean
    echo Begin to rebuild the Plugin >> %BUILD_LOG%
    echo %DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv

    %DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

    set res=%errorlevel%
    type %PLUGIN_FRAMEWORK_PATH%Windows_Compile\build.log
    if %res% NEQ 0 (
        echo compile error, pls check
        exit 1 
    )
    echo End to build x64 Plugin
goto :eof


rem
rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
rem
:FindVCTOOLHome
    set VCTOOLTemp=n/a
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        set	VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
        echo Find vswhere.exe suceess. >> %BUILD_LOG%
        echo Find vswhere.exe success.
    ) else (
        echo Not find vswhere.exe. >> %BUILD_LOG%
        echo Not find vswhere.exe.
    )
        
    rem Query the VS2017 installationPath with vswhere.exe
    for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set VCTOOLTemp=%%i
    )

    if "%VCTOOLTemp%" NEQ "n/a" (set %1="%VCTOOLTemp%")
goto :eof

rem
rem Delete a file
rem
:DeleteFile
    set FileName=%~1
    if exist "%FileName%" (del /f /q "%FileName%")
goto :eof

:FileNotFound
    echo  %DATE% %TIME% %1 Not Found,Compile Failed. >> %BUILD_LOG%
    exit 1
goto :eof
     echo "Finish"
     Exit