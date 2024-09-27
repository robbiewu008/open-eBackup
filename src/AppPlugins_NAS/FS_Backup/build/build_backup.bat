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
if "%Module_Branch%" == "" (
    set Module_Branch=develop_backup_software_1.6.0RC1
)
set BACKUP_PATH=%CurBatPath%..\..\
echo BACKUP_PATH is %BACKUP_PATH%

rem create windows compile folder
cd "%CurBatPath%"
if exist %BACKUP_PATH%Windows_Compile  rd /S /Q "%BACKUP_PATH%Windows_Compile"
IF not EXIST %BACKUP_PATH%Windows_Compile   MKDIR  %BACKUP_PATH%Windows_Compile
IF not EXIST %BACKUP_PATH%Windows_Compile\vsprj   MKDIR  %BACKUP_PATH%Windows_Compile\vsprj
IF not EXIST %BACKUP_PATH%Windows_Compile\Module   MKDIR  %BACKUP_PATH%Windows_Compile\Module
IF not EXIST %BACKUP_PATH%Windows_Compile\Backup   MKDIR  %BACKUP_PATH%Windows_Compile\Backup
IF not EXIST %BACKUP_PATH%Windows_Compile\vsprj\bin   MKDIR  %BACKUP_PATH%Windows_Compile\vsprj\bin

@REM rem Copy external pkg

xcopy /y /e %BACKUP_PATH%\FS_Backup\vsprj  %BACKUP_PATH%\Windows_Compile\vsprj



echo d | xcopy /y /e %BACKUP_PATH%\FS_Backup\src  %BACKUP_PATH%\Windows_Compile\FS_Backup\src



echo d | xcopy /y /e %BACKUP_PATH%\FS_Backup\tool  %BACKUP_PATH%\Windows_Compile\FS_Backup\tool
xcopy /y /e %BACKUP_PATH%\Module  %BACKUP_PATH%\Windows_Compile\Module
call %BACKUP_PATH%\Windows_Compile\Module\build\download_3rd.bat
call %BACKUP_PATH%\Windows_Compile\Module\build\download_module_from_cmc.bat %Module_Branch%
echo d | xcopy /y /e %BACKUP_PATH%\Windows_Compile\Module\libs %BACKUP_PATH%\Windows_Compile\vsprj\bin\bin
cd "%BACKUP_PATH%Windows_Compile%"
git-bash.exe -c "cd FS_Backup/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"
git-bash.exe -c "cd Module/src; find -name *.h | xargs unix2dos;"

set SOLUTION=%BACKUP_PATH%Windows_Compile\vsprj\build\Backup.sln
IF not EXIST %SOLUTION%  call :FileNotFound

echo SOLUTION is %SOLUTION%

set BUILD_LOG="%BACKUP_PATH%Windows_Compile\build.log"
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


rem Copy lib into Backup/libs
IF EXIST %BACKUP_PATH%\FS_Backup\libs  rd /S /Q "%BACKUP_PATH%\FS_Backup\libs"
IF not EXIST %BACKUP_PATH%\FS_Backup\libs MKDIR  %BACKUP_PATH%\FS_Backup\libs
COPY /Y %BACKUP_PATH%Windows_Compile\vsprj\bin\bin\*.dll      %BACKUP_PATH%\FS_Backup\libs
COPY /Y %BACKUP_PATH%Windows_Compile\vsprj\bin\bin\*.lib      %BACKUP_PATH%\FS_Backup\libs
COPY /Y %BACKUP_PATH%Windows_Compile\vsprj\bin\bin\*.exe      %BACKUP_PATH%\FS_Backup\libs

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
    type %BACKUP_PATH%Windows_Compile\build.log
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
