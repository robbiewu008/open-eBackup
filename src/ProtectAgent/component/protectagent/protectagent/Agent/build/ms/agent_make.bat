::  This file is a part of the open-eBackup project.
::  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
::  If a copy of the MPL was not distributed with this file, You can obtain one at
::  http://mozilla.org/MPL/2.0/.
:: 
::  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
:: 
::  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
::  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
::  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.

@echo off
setlocal EnableDelayedExpansion

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set AGENTPATH=%CurBatPath%..\..\
set SOLUTION=%AGENTPATH%vsprj\agentprj\agentprj.sln
set SDP_SOLUTION=%AGENTPATH%vsprj\agentprj\sdpngx\sdpngx.sln

echo AGENTPATH is %AGENTPATH%
echo SOLUTION is %SOLUTION%

set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

set BUILD_LOG="%CurBatPath%build.log"
set VCVARS32_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvars32.bat
echo VCVARS32_BAT is %VCVARS32_BAT% >> %BUILD_LOG%

set VCVARSX86_AMD64_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvarsx86_amd64.bat
echo VCVARSX86_AMD64_BAT is %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%

set DEVEVN_FILE=%VCTOOL_HOME%\Common7\IDE\devenv.exe
echo DEVEVN_FILE is %DEVEVN_FILE% >> %BUILD_LOG%

echo include is %INCLUDE%

call :Create_windows_python_executalbe_file
if not "!errorlevel!" == "0" (
    exit /b 1
)

echo 1 is "%1"

if "%1" EQU "Clean32Openssl" (call :Clean32Openssl)
if "%1" EQU "Clean64Openssl" (call :Clean64Openssl)

if "%1" EQU "compile32agent" (
    call :Compile32Agent
) else if "%1" EQU "compile64agent" (
    call :Compile64Agent
)
if not "!errorlevel!" == "0" (
    echo "Echo build log."
    type %BUILD_LOG% | findstr /N .
    echo "---------------Start filter error log---------------."
    findstr /N /I "error" %BUILD_LOG% | findstr /V "errorcode"
    echo "---------------End filter error log---------------."
    exit /b 1
)
echo End of Compile
goto :eof

:Create_windows_python_executalbe_file
    set wexpectLocation=
    for /f "tokens=2" %%i in ('pip show wexpect ^| findstr /c:"Location"') do (
        set wexpectLocation=%%i
    )
    set wexpectLocation=!wexpectLocation!\wexpect
    set wexpectDist=!wexpectLocation!\dist
    echo !wexpectLocation! !wexpectDist!

    if not exist "!wexpectDist!" (
        cd /d !wexpectLocation!
        pyinstaller __main__.py -n wexpect > nul
        if not exist "!wexpectDist!" (
            echo "Not Create wexpect execute file."
            exit /b 1
        )
    )
    cd /d %AGENTPATH%\bin\bat
    pyinstaller --onefile --add-data "!wexpectDist!;." -F CreateDataturbolink.py >nul
    if not exist %AGENTPATH%\bin\bat\dist\CreateDataturbolink.exe (
        echo "Not Create Dataturbo link execute file."
        exit /b 1
    )
    echo "Create Dataturbo link execute file success."

    cd /d %AGENTPATH%\bin\bat
    pyinstaller --onefile --add-data "!wexpectDist!;." -F DataturboUmount.py >nul
    if not exist %AGENTPATH%\bin\bat\dist\DataturboUmount.exe (
        echo "Not Create Dataturbo umount execute file."
        exit /b 1
    )
    echo "Create Dataturbo umount execute file success."
    exit /b 0
goto :eof

:Clean32Openssl
	rem clean openssl 32 lib file
	rem setting the enviroment with VC command 32
	set SOLUTION_CONFIG="Release|Win32"
	set ACTION=Rebuild
	call %VCVARS32_BAT%


	%Drive%
	cd %AGENTPATH%open_src\openssl
	perl configure VC-WIN32 > %BUILD_LOG%
	call ms\do_ms
	echo call vsvar
	nmake -f ms/nt.mak clean >> %BUILD_LOG%

	echo End to Clean 32 Openssl
goto :eof

:Clean64Openssl
	rem clean openssl 64 lib file

	rem setting the enviroment with VC command amd64
	call %VCVARSX86_AMD64_BAT%

	%Drive%
	cd %AGENTPATH%open_src\openssl
	rem Adding the 3 lines below to avoid NMAKE : fatal error U1077: 'del' : return code '0x1'
	IF NOT EXIST %AGENTPATH%open_src\openssl\inc32 MKDIR  %AGENTPATH%open_src\openssl\inc32
	IF NOT EXIST %AGENTPATH%open_src\openssl\out32 MKDIR  %AGENTPATH%open_src\openssl\out32
	IF NOT EXIST %AGENTPATH%open_src\openssl\tmp32 MKDIR  %AGENTPATH%open_src\openssl\tmp32

	perl configure VC-WIN64A > %BUILD_LOG%
	call ms\do_win64a

	nmake -f ms/nt.mak clean >> %BUILD_LOG%
	echo End to Clean 64 Openssl
goto :eof

:Compile32Agent
	rem begin to make openssl 32 lib file
	rem setting the enviroment with VC command 32
	set SOLUTION_CONFIG="Release|Win32"
	set ACTION=Rebuild
	call %VCVARS32_BAT%

	%Drive%
	cd %AGENTPATH%open_src\openssl
	perl configure VC-WIN32 >> %BUILD_LOG%
	call ms\do_ms
	echo call vsvar
	rem nmake -f ms/nt.mak clean >> %BUILD_LOG%
	nmake -f ms\nt.mak >> %BUILD_LOG%

	rem make nginx
	cd %AGENTPATH%open_src\nginx_tmp
	nmake -f objs/Makefile >> %BUILD_LOG%

	rem make sqlite3.c file
	cd %AGENTPATH%open_src\sqlite
	call nmake /f Makefile.msc sqlite3.c clean >> %BUILD_LOG%
	call nmake /f Makefile.msc sqlite3.c >> %BUILD_LOG%

	cd %CurBatPath%

	if not exist %AGENTPATH%open_src\openssl\out32\lib\libeay32.lib (goto CompileFailed)
	if not exist %AGENTPATH%open_src\openssl\out32\lib\ssleay32.lib (GOTO CompileFailed)

	echo Begin to clean compile >> %BUILD_LOG%
	echo %DEVEVN_FILE% "%SOLUTION%" /clean
	%DEVEVN_FILE% "%SOLUTION%" /clean

	echo Begin to rebuild the Agent >> %BUILD_LOG%
	echo %DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv

	%DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

	echo End to build x32 Agent
goto :eof

:Compile64Agent
    %Drive%
    call %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%
    echo Generate AgentDB.db
    call :GenerateAgentDB

    echo Begin Generate Thrift cpp
    call :GenerateThriftCPP

    rem build src
    cd %CurBatPath%
    set SOLUTION_CONFIG="Release|x64"
    set ACTION=Rebuild
    echo Begin to clean compile >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SOLUTION%" /clean

    %DEVEVN_FILE% "%SOLUTION%" /clean
    echo Begin to rebuild the Agent >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv

    %DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%
	if not "!errorlevel!" == "0" (
		echo "Compile fail."
		exit /b 1
	)

    echo End to build x64 Agent
goto :eof

rem 
rem Generate AgentDB.db
rem 
:GenerateAgentDB
    cd %AGENTPATH%open_src\sqlite
    IF NOT EXIST %AGENTPATH%open_src\sqlite\sqlite3.exe (call :FileNotFound sqlite3.exe)
    if exist %AGENTPATH%selfdevelop\AgentDB.db (
        del %AGENTPATH%selfdevelop\AgentDB.db 1>NUL 2>NUL
    )
	sqlite3.exe %AGENTPATH%selfdevelop\AgentDB.db < %AGENTPATH%build\create_table.sql
	IF NOT EXIST  %AGENTPATH%selfdevelop\AgentDB.db (call :FileNotFound AgentDB.db)
goto :eof

rem
rem Generate Thrift CPP
rem
:GenerateThriftCPP
    if not exist %AGENTPATH%open_src\thrift\build\thrift.exe (call :FileNotFound sqlite3.exe)
    xcopy /y /e %AGENTPATH%open_src\thrift\build\thrift.exe %AGENTPATH%src\src\servicecenter\thrift\
    
    cd %AGENTPATH%src\src\servicecenter\thrift
    thrift.exe -o . -r -gen cpp .\ApplicationProtectBaseDataType.thrift
    if not "!errorlevel!" == "0" (exit /b 1)
    thrift.exe -o . -r -gen cpp .\ApplicationProtectFramework.thrift
    if not "!errorlevel!" == "0" (exit /b 1)
    thrift.exe -o . -r -gen cpp .\ApplicationProtectPlugin.thrift
    if not "!errorlevel!" == "0" (exit /b 1)
    
    del /q .\gen-cpp\*skeleton.cpp
    move /y .\gen-cpp\*.cpp %AGENTPATH%src\src\apps\appprotect\plugininterface
    move /y .\gen-cpp\*.h %AGENTPATH%src\inc\apps\appprotect\plugininterface
    
    echo GenerateThriftCPP success.
goto :eof

rem
rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
rem
:FindVCTOOLHome
    set VCTOOLTemp=n/a

    IF EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        set	"VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
        echo Find vswhere.exe suceess. >> %BUILD_LOG%
        echo Find vswhere.exe success.
    ) ELSE (
        echo Not find vswhere.exe. >> %BUILD_LOG%
        echo Not find vswhere.exe.
    )
        
    rem Query the VS2017 installationPath with vswhere.exe
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
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

:Exit
