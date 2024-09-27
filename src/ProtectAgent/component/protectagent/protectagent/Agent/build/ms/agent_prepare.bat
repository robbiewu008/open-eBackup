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

rem 使用7z解压时，现将*gz格式解压为*tar格式，然后再次解压，jsoncpp是解压至\dist目录下
set Drive=%~d0
set CURBAT_PATH=%~dp0

set AGENT_ROOT_PATH=%CURBAT_PATH%..\..\
set AGENT_OPEN_SRC_PATH=%AGENT_ROOT_PATH%open_src

set OPEN_SRC_FCGI_DIR=fcgi
set OPEN_SRC_JSONCPP_DIR=jsoncpp
set OPEN_SRC_NGINX_DIR=nginx_tmp
set OPEN_SRC_OPENSSL_DIR=openssl
set OPEN_SRC_SQLITE_DIR=sqlite
set OPEN_SRC_SNMP_DIR=snmp++
set OPEN_SRC_TINYXML_DIR=tinyxml
set OPEN_SRC_CURL_DIR=curl
set OPEN_SRC_BOOST_DIR=boost
set OPEN_SRC_THRIFT_DIR=thrift
set OPEN_SRC_LIBEVENT_DIR=libevent
set OPEN_SRC_ZLIB_DIR=zlib

call :Check_open_src_packages
call :PrepareFcgi

if not exist %AGENT_OPEN_SRC_PATH%\libevent\libevent.lib (
    call :PrepareLibevent
)

if not exist %AGENT_OPEN_SRC_PATH%\thrift\build\thrift.exe (
    call :PrepareThrift
)

call :PrepareJsoncpp

endlocal
goto :EOF
    
:PrepareFcgi
    if exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c1 del /f /q %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c1
    
    set /a n=0
    set str=
    for /f "delims=" %%i in ('findstr /n .* %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c') do (
        set "str=%%i"
        set /a "n=n+1"
        setlocal EnableDelayedExpansion
        set str=!str:*:=!
		
        echo.!str!>>%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c1
        if !n!==719 ( 
            set "newstr=        sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");"
            echo.!newstr!>>%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c1
        )
        endlocal
    )

    copy /y %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c.bak
    move /y %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c1 %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\libfcgi\os_win32.c
goto :EOF

:PrepareLibevent
    setlocal EnableDelayedExpansion
    set ParamFile=%AGENT_OPEN_SRC_PATH%\libevent\WIN32-Code\nmake\event2\event-config.h
    set ParamFileBak=!ParamFile!.bak
    (for /f "tokens=1* delims=: eol=" %%a in ('findstr /n .* "!ParamFile!"') do (
        set str=%%b
        if "!str!" == "" (
            echo=
        ) ELSE if "!str!" == "/* #define EVENT__HAVE_STDINT_H 1 */" (
            echo #define EVENT__HAVE_STDINT_H 1
        ) ELSE (
            echo !str!
        )
    ))>!ParamFileBak!
    move /y !ParamFileBak! !ParamFile!
    copy /y !ParamFile! %AGENT_OPEN_SRC_PATH%\libevent\include\event2
    endlocal
goto :EOF

:PrepareThrift
    setlocal EnableDelayedExpansion
    set HeadFile=%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\lib\cpp\src\thrift\thrift_export.h
    set HeadFileBak=!HeadFile!.bak
    (for /f "tokens=1* delims=: eol=" %%a in ('findstr /n .* "!HeadFile!"') do (
        set num=%%a
        set str=%%b
        if "!num!" == "3" (
            echo #define THRIFT_EXPORT
        ) else if "!str!" == "" (
            echo=
        ) else (
            echo !str!
        )
    ))>!HeadFileBak!
    move /y !HeadFileBak! !HeadFile!
    copy /y %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\lib\cpp\src\thrift\windows\config.h %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\lib\cpp\src\thrift
    set ParamFile=%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\lib\cpp\src\thrift\config.h
    set ParamFileBak=!ParamFile!.bak
    (for /f "tokens=1* delims=: eol=" %%a in ('findstr /n .* "!ParamFile!"') do (
        set num=%%a
        set str=%%b
        if "!num!" == "58" (
            echo=
            echo /* Define to the version of this package. */
            echo #define PACKAGE_VERSION "0.14.0"
            echo=
        ) else if "!str!" == "" (
            echo=
        ) else (
            echo !str!
        )
    ))>!ParamFileBak!
    move /y !ParamFileBak! !ParamFile!

    cd %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\compiler\cpp
    flex -osrc\thrift\thriftl.cc src\thrift\thriftl.ll
    bison -y -o "src/thrift/thrifty.cc" --defines src/thrift/thrifty.yy
    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\compiler\cpp\src\thrift\thriftl.cc (
        echo "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\compiler\cpp\src\thrift\thriftl.cc not exist."
        exit 1
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\compiler\cpp\src\thrift\thrifty.cc (
        echo "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\compiler\cpp\src\thrift\thrifty.cc not exist."
        exit 1
    )

    if exist %AGENT_OPEN_SRC_PATH%\patch\thrift_windows.patch (
        patch -N %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%\compiler\cpp\src\thrift\thriftl.cc < %AGENT_OPEN_SRC_PATH%\patch\thrift_windows.patch
    )

    endlocal
goto :EOF

:PrepareJsoncpp
	IF EXIST %AGENT_OPEN_SRC_PATH%\patch\json_cpp_reader_h_parse.patch (
       patch -N %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_JSONCPP_DIR%\include\json\reader.h < %AGENT_OPEN_SRC_PATH%\patch\json_cpp_reader_h_parse.patch
    )
	IF EXIST %AGENT_OPEN_SRC_PATH%\patch\json_cpp_reader_cpp_parse.patch (
       patch -N %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_JSONCPP_DIR%\src\lib_json\json_reader.cpp < %AGENT_OPEN_SRC_PATH%\patch\json_cpp_reader_cpp_parse.patch
    )
goto :EOF

:Check_open_src_packages
    echo "Checking the open source package. Please wait..."
    setlocal EnableDelayedExpansion
    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_JSONCPP_DIR% (
        if not exist %AGENT_OPEN_SRC_PATH%\jsoncpp_00.11.0 (
            echo "OpenSrc package[jsoncpp_00.11.0] is not exist."
            exit 1
        )
        xcopy /y /e %AGENT_OPEN_SRC_PATH%\jsoncpp_00.11.0 %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_JSONCPP_DIR%\
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR% (
        if not exist %AGENT_OPEN_SRC_PATH%\fcgi2 (
            echo "OpenSrc package[%AGENT_OPEN_SRC_PATH%\fcgi2] is not exist."
            exit 1
        )
        xcopy /y /e %AGENT_OPEN_SRC_PATH%\fcgi2 %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_NGINX_DIR% (
        if not exist %AGENT_OPEN_SRC_PATH%\nginx (
            echo "OpenSrc package[nginx] is not exist."
            exit 1
        )

        xcopy /y /e %AGENT_OPEN_SRC_PATH%\nginx %AGENT_OPEN_SRC_PATH%\nginx_back\
        move /y %AGENT_OPEN_SRC_PATH%\nginx %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_NGINX_DIR%
        md  %AGENT_OPEN_SRC_PATH%\nginx
        xcopy /y /e %AGENT_OPEN_SRC_PATH%\objs_ngx %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_NGINX_DIR%\
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_SQLITE_DIR% (
        echo "OpenSrc package[%OPEN_SRC_SQLITE_DIR%] is not exist."
        exit 1
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_SNMP_DIR% (
        if not exist %AGENT_OPEN_SRC_PATH%\SNMP (
            echo "OpenSrc package[SNMP] is not exist."
            exit 1
        )
        xcopy /y /e %AGENT_OPEN_SRC_PATH%\SNMP %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_SNMP_DIR%\
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_TINYXML_DIR% (
        if not exist %AGENT_OPEN_SRC_PATH%\tinyxml2 (
            echo "OpenSrc package[tinyxml2] is not exist."
            exit 1
        )
        xcopy /y /e %AGENT_OPEN_SRC_PATH%\tinyxml2 %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_TINYXML_DIR%\
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_BOOST_DIR% (
        echo "OpenSrc package[%OPEN_SRC_BOOST_DIR%] is not exist."
        exit 1
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR% (
        echo "OpenSrc package[%OPEN_SRC_THRIFT_DIR%] is not exist."
        exit 1
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_LIBEVENT_DIR% (
        echo "OpenSrc package[%OPEN_SRC_LIBEVENT_DIR%] is not exist."
        exit 1
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_ZLIB_DIR% (
        echo "OpenSrc package[%OPEN_SRC_ZLIB_DIR%] is not exist."
        exit 1
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_OPENSSL_DIR% (
        echo "OpenSrc package[%OPEN_SRC_OPENSSL_DIR%] is not exist."
        exit 1
    ) else (
        if not exist %AGENT_OPEN_SRC_PATH%\openssl_temp (
            xcopy /y /e %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_OPENSSL_DIR% %AGENT_OPEN_SRC_PATH%\openssl_temp\
        )
    ) 

    endlocal
goto :EOF


