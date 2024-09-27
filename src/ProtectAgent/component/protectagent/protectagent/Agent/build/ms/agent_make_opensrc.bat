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
setlocal DisableDelayedExpansion

set Drive=%~d0
set CURBAT_PATH=%~dp0

set AGENT_ROOT_PATH=%CURBAT_PATH%..\..\
set AGENT_OPEN_SRC_PATH=%AGENT_ROOT_PATH%open_src
set BUILD_LOG=%CURBAT_PATH%build.log
set OPEN_SRC_7ZIP_DIR=7zip

set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

set VCVARS32_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvars32.bat
echo VCVARS32_BAT is %VCVARS32_BAT% >> %BUILD_LOG%

set VCVARSX86_AMD64_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvarsx86_amd64.bat
echo VCVARSX86_AMD64_BAT is %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%

set DEVEVN_FILE=%VCTOOL_HOME%\Common7\IDE\devenv.exe
echo DEVEVN_FILE is %DEVEVN_FILE% >> %BUILD_LOG%

call %CURBAT_PATH%agent_prepare.bat
call :Build_Open_Src
endlocal
goto :EOF

:Build_Open_Src
    call %VCVARS32_BAT% >> %BUILD_LOG%
    if not exist %AGENT_OPEN_SRC_PATH%\nginx_tmp\objs\nginx.exe (
        echo Begin build 32 bit openssl for 32 bit nginx
        call :BuildNginx
    )

    call %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%
    if not exist %AGENT_OPEN_SRC_PATH%\openssl\out32\lib\libssl.lib (
        echo begin build OpenSSL
        call :BuildOpenSSL
    )

    if not exist %AGENT_OPEN_SRC_PATH%\sqlite\sqlite3.exe (
        echo Begin build BuildSqlite3
        call :BuildSqlite3
    )

    if not exist %AGENT_OPEN_SRC_PATH%\curl\builds\bin\bin\libcurl.dll (
        echo Begin build Curl
        call :BuildCurl
    )

    if not exist %AGENT_OPEN_SRC_PATH%\curl\builds\out_static\lib\libcurl_a.lib (
        echo The curl static library does not exist.
        call :FileNotFound libcurl_a.lib
    )

    if not exist %AGENT_OPEN_SRC_PATH%\libevent\libevent.lib (
        echo Begin build Libevent
        call :BuildLibevent
    )

    if not exist %AGENT_OPEN_SRC_PATH%\zlib\zdll.lib (
        echo Begin build Zlib
        call :BuildZlib
    )

    if not exist %AGENT_OPEN_SRC_PATH%\boost\b2.exe (
        echo Begin build Boost
        call :BuildBoost
    )

    if not exist %AGENT_OPEN_SRC_PATH%\thrift\build\thrift.exe (
        echo Begin build Thrift
        call :BuildThriftCompiler
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\lib\7z.exe (
        echo Begin build 7zip
        call :Build7Zip
    ) else (
        echo Copy 7zip to third_party_groupware
        call :Copy7zip
    )

endlocal
goto :EOF

rem
rem build nginx
rem
:BuildNginx
    cd %AGENT_OPEN_SRC_PATH%\nginx_tmp
    nmake -f auto/lib/openssl/makefile.msvc OPENSSL="../openssl" OPENSSL_OPT="no-asm" >> %BUILD_LOG%
    nmake -f objs/Makefile >> %BUILD_LOG%
    IF NOT EXIST %AGENT_OPEN_SRC_PATH%\nginx_tmp\objs\nginx.exe (call :FileNotFound nginx.exe)
    
    echo BuildNginx success.
goto :eof

rem
rem build openssl
rem
:BuildOpenSSL
    RD /Q /S %AGENT_OPEN_SRC_PATH%\openssl
    xcopy /y /e %AGENT_OPEN_SRC_PATH%\openssl_temp %AGENT_OPEN_SRC_PATH%\openssl\
    cd %AGENT_OPEN_SRC_PATH%\openssl

    perl configure VC-WIN64A no-shared no-asm no-tests --prefix="%AGENT_OPEN_SRC_PATH%\openssl\out32" --openssldir="%AGENT_OPEN_SRC_PATH%\openssl\out32\ssl" >> %BUILD_LOG%
    nmake >> %BUILD_LOG%
    nmake install_sw>> %BUILD_LOG%
    if not exist %AGENT_OPEN_SRC_PATH%\openssl\out32\lib\libssl.lib (call :FileNotFound libssl.lib)
    if not exist %AGENT_OPEN_SRC_PATH%\openssl\out32\lib\libcrypto.lib (call :FileNotFound libcrypto.lib)
    
    echo BuildOpenSSL success.
goto :eof

rem
rem build libevent
rem
:BuildLibevent
    cd %AGENT_OPEN_SRC_PATH%\libevent
    nmake /f Makefile.nmake static_libs >> %BUILD_LOG%
    if not exist %AGENT_OPEN_SRC_PATH%\libevent\libevent.lib (call :FileNotFound libevent.lib)
    if not exist %AGENT_OPEN_SRC_PATH%\libevent\libevent_core.lib (call :FileNotFound libevent_core.lib)
    if not exist %AGENT_OPEN_SRC_PATH%\libevent\libevent_extras.lib (call :FileNotFound libevent_extras.lib)
    
    echo BuildLibevent success.
goto :eof

rem
rem build sqlite3.exe & AgentDB.db
rem
:BuildSqlite3
    cd %AGENT_OPEN_SRC_PATH%\sqlite
    nmake /f .\Makefile.msc sqlite3.exe TOP=.\ >> %BUILD_LOG%
    IF NOT EXIST %AGENT_OPEN_SRC_PATH%\sqlite\sqlite3.exe (call :FileNotFound sqlite3.exe)
    echo BuildSqlite3 success.
goto :eof

rem
rem build zlib
rem
:BuildZlib
    cd %AGENT_OPEN_SRC_PATH%\zlib\contrib\masmx64
    call bld_ml64.bat >> %BUILD_LOG%
    if not exist %AGENT_OPEN_SRC_PATH%\zlib\contrib\masmx64\gvmat64.obj (call :FileNotFound gvmat64.obj)
    if not exist %AGENT_OPEN_SRC_PATH%\zlib\contrib\masmx64\inffasx64.obj (call :FileNotFound inffasx64.obj)

    cd %AGENT_OPEN_SRC_PATH%\zlib
    copy /y %AGENT_OPEN_SRC_PATH%\zlib\contrib\masmx64\gvmat64.obj %AGENT_OPEN_SRC_PATH%\zlib\gvmat64.obj
    copy /y %AGENT_OPEN_SRC_PATH%\zlib\contrib\masmx64\inffasx64.obj %AGENT_OPEN_SRC_PATH%\zlib\inffasx64.obj
    copy /y %AGENT_OPEN_SRC_PATH%\zlib\contrib\masmx64\inffas8664.c %AGENT_OPEN_SRC_PATH%\zlib\inffas8664.c
    call nmake -f win32/Makefile.msc AS=ml64 LOC="-DASMINF -I." OBJA="inffasx64.obj gvmat64.obj inffas8664.obj" >> %BUILD_LOG%
    
    if not exist %AGENT_OPEN_SRC_PATH%\zlib\zdll.lib (call :FileNotFound zdll.lib)
    if not exist %AGENT_OPEN_SRC_PATH%\zlib\zlib.lib (call :FileNotFound zlib.lib)
    if not exist %AGENT_OPEN_SRC_PATH%\zlib\zlib1.dll (call :FileNotFound zlib1.dll)
    
    echo BuildZlib success.
goto :eof

rem
rem build boost
rem
:BuildBoost
    cd %AGENT_OPEN_SRC_PATH%\boost
    call bootstrap.bat
    if not exist %AGENT_OPEN_SRC_PATH%\boost\bootstrap.bat (call :FileNotFound bootstrap.bat)

    call b2.exe variant=release threading=multi threadapi=win32 link=shared runtime-link=shared --prefix= %AGENT_OPEN_SRC_PATH%\boost address-model=64 architecture=x86 install
    if not exist %AGENT_OPEN_SRC_PATH%\boost\b2.exe (call :FileNotFound b2.exe)
    if not exist %AGENT_OPEN_SRC_PATH%\boost\stage\lib (call :FileNotFound lib)
    
    echo BuildBoost success.
goto :eof

rem
rem build curl
rem
:BuildCurl
    rd /S /Q %AGENT_OPEN_SRC_PATH%\openssl\lib 1>nul 2>nul
    md %AGENT_OPEN_SRC_PATH%\openssl\lib 1>nul 2>nul
    XCOPY /Y /E %AGENT_OPEN_SRC_PATH%\openssl\out32\lib %AGENT_OPEN_SRC_PATH%\openssl\lib 1>nul 2>nul
    
    cd /d %AGENT_OPEN_SRC_PATH%\curl\
    call buildconf.bat

    cd /d %AGENT_OPEN_SRC_PATH%\curl\winbuild
    rd /S /Q %AGENT_OPEN_SRC_PATH%\curl\builds\bin
    nmake /f Makefile.vc mode=dll MACHINE=x64 DEBUG=no WITH_SSL=static SSL_PATH=%AGENT_OPEN_SRC_PATH%\openssl\ WITH_PREFIX=%AGENT_OPEN_SRC_PATH%\curl\builds\bin
    if not exist %AGENT_OPEN_SRC_PATH%\curl\builds\bin\bin\libcurl.dll (call :FileNotFound libcurl.dll)

    echo Buildcurl success.
goto :eof

rem
rem build 7zip 
rem
:Build7Zip
    cd /d %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%
    7z.exe x -y 7z2301-src.7z -o%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%

    cd /d %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\CPP\7zip\UI\Console
    call %VCVARS64_BAT% >> %BUILD_LOG%
    nmake NEW_COMPILER=1 MY_STATIC_LINK=1
    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\CPP\7zip\UI\Console\x64\7z.exe (call :FileNotFound 7z.exe)

    cd /d %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\CPP\7zip\Bundles\Format7zF
    call %VCVARS64_BAT% >> %BUILD_LOG%
    nmake NEW_COMPILER=1 MY_STATIC_LINK=1
    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\CPP\7zip\Bundles\Format7zF\x64\7z.dll (call :FileNotFound 7z.dll)

    md %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\lib 1>nul 2>nul
    COPY /Y /E %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\CPP\7zip\UI\Console\x64\7z.exe %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\lib 1>nul 2>nul
    COPY /Y /E %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\CPP\7zip\Bundles\Format7zF\x64\7z.dll %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR%\lib 1>nul 2>nul

    echo Copy 7zip to third_party_groupware
    call :Copy7zip

    echo Build7zip success.
goto :eof


rem
rem copy 7zip 
rem
:Copy7zip
    MKDIR %AGENT_ROOT_PATH%\third_party_groupware\7Zip
    copy /y %AGENT_OPEN_SRC_PATH%\7zip\lib\7z.exe %AGENT_ROOT_PATH%\third_party_groupware\7Zip
    copy /y %AGENT_OPEN_SRC_PATH%\7zip\lib\7z.dll %AGENT_ROOT_PATH%\third_party_groupware\7Zip

    echo Copy7zip success.
goto :eof


:BuildThriftCompiler 
    echo  %DATE% %TIME% %1 begin compile ThriftCompiler >> %BUILD_LOG%
    set ACTION1=Rebuild
    set SOLUTION_CONFIG="Release"
    set ACTION2=Projectconfig
    set PROJECT_CONFIG="Release|x64"
    set THRIFTEXE=%AGENT_ROOT_PATH%vsprj\agentprj\thrift\compiler\compiler.sln
    cd %AGENT_ROOT_PATH%vsprj\agentprj\thrift\compiler
    
    echo Begin to clean compile >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%THRIFTEXE%" /clean
    %DEVEVN_FILE% "%THRIFTEXE%" /clean
    
    echo %DEVEVN_FILE% "%THRIFTEXE%" /%ACTION1% %SOLUTION_CONFIG% /%ACTION2% %PROJECT_CONFIG% 
    %DEVEVN_FILE% "%THRIFTEXE%" /%ACTION1% %SOLUTION_CONFIG% /%ACTION2% %PROJECT_CONFIG% /Out %BUILD_LOG%
    
    if not exist %AGENT_OPEN_SRC_PATH%\thrift\build\thrift.exe (call :FileNotFound thrift.exe)
    echo BuildThriftCompiler success.
goto :eof

:FindVCTOOLHome
    set VCTOOLTemp=n/a

    IF EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        set    "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
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

:FileNotFound
    echo  %DATE% %TIME% %1 Not Found,Compile Failed. >> %BUILD_LOG%
    exit 1
goto :eof
