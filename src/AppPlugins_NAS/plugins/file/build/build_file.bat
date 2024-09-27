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
echo CurBatPath is %CurBatPath%

set PLUGIN_PATH=%CurBatPath%..\..\..\
echo PLUGIN_PATH is %PLUGIN_PATH%
if "%Module_Branch%" == "" (
    set Module_Branch=develop_backup_software_1.6.0RC1
)
if "%Scanner_Branch%" == "" (
    set Scanner_Branch=CI_opt
)
if "%Backup_Branch%" == "" (
    set Backup_Branch=CI_opt
)
rem create windows compile folder
cd "%CurBatPath%"
@REM if exist %PLUGIN_PATH%Windows_Compile  rd /S /Q "%PLUGIN_PATH%Windows_Compile"
IF not EXIST %PLUGIN_PATH%Windows_Compile\plugins   MKDIR  %PLUGIN_PATH%Windows_Compile\plugins
call %PLUGIN_PATH%\framework\build\build.bat

@REM rem Copy external pkg
xcopy /y /e %PLUGIN_PATH%\plugins\file\vsprj  %PLUGIN_PATH%\Windows_Compile\vsprj
xcopy /y /e %PLUGIN_PATH%\FS_Scanner\vsprj  %PLUGIN_PATH%\Windows_Compile\vsprj
xcopy /y /e %PLUGIN_PATH%\FS_Backup\vsprj  %PLUGIN_PATH%\Windows_Compile\vsprj
echo d | xcopy /y /e %PLUGIN_PATH%\FS_Backup  %PLUGIN_PATH%\Windows_Compile\FS_Backup
echo d | xcopy /y /e %PLUGIN_PATH%\FS_Scanner  %PLUGIN_PATH%\Windows_Compile\FS_Scanner
echo d | xcopy /y /e %PLUGIN_PATH%\plugins\file  %PLUGIN_PATH%\Windows_Compile\plugins\file

@REM download module、Scanner、Backup from cmc

@REM call %PLUGIN_PATH%\Windows_Compile\FS_Scanner\build\download_scanner_from_cmc.bat %Scanner_Branch%
@REM call %PLUGIN_PATH%\Windows_Compile\FS_Backup\build\download_backup_from_cmc.bat %Backup_Branch%
@REM echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\FS_Scanner\libs %PLUGIN_PATH%\Windows_Compile\vsprj\bin\bin
@REM echo d | xcopy /y /e %PLUGIN_PATH%\Windows_Compile\FS_Backup\libs %PLUGIN_PATH%\Windows_Compile\vsprj\bin\bin

cd "%PLUGIN_PATH%Windows_Compile%"
git-bash.exe -c "cd FS_Backup/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"
git-bash.exe -c "cd plugins/file/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"
git-bash.exe -c "cd FS_Scanner/localhost_src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"

set SOLUTION=%PLUGIN_PATH%Windows_Compile\vsprj\build\Plugin.sln
IF not EXIST %SOLUTION%  call :FileNotFound

echo SOLUTION is %SOLUTION%

set BUILD_LOG="%PLUGIN_PATH%Windows_Compile\build.log"
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

call :Compile64PluginFramework


rem Copy lib into plugins/file/lib
IF EXIST %PLUGIN_PATH%\plugins\file\lib  rd /S /Q "%PLUGIN_PATH%\plugins\file\lib"
IF not EXIST %PLUGIN_PATH%\plugins\file\lib MKDIR  %PLUGIN_PATH%\plugins\file\lib
COPY /Y %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.dll      %PLUGIN_PATH%\plugins\file\lib
COPY /Y %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.exe      %PLUGIN_PATH%\plugins\file\lib
COPY /Y %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.pdb      %PLUGIN_PATH%\plugins\file\lib

echo End of Compile
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
    type %PLUGIN_PATH%Windows_Compile\build.log
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
