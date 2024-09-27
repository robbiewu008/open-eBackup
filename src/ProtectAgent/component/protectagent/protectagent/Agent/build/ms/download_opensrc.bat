@echo off
setlocal EnableDelayedExpansion

set CurBatPath=%~dp0
if '%OPENSOURCE_BRANCH%'=='' (set OPENSOURCE_BRANCH=master_backup_software_1.6.0RC1)
echo OPENSOURCE_BRANCH is %OPENSOURCE_BRANCH%
set SYS_NAME=Windows2012
set SYS_ARCH=x64
set Version=1.2.1RC1

set AGENT_OPEN_SRC_PATH=%CurBatPath%../../open_src
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
set OPEN_SRC_7ZIP_DIR=7zip
set OPEN_SRC_3RD_DIR=%CurBatPath%../../../../../../../open-source-obligation/ThirdParty/%SYS_NAME%/%SYS_ARCH%/third_party_groupware

set Type=%~1
if "%Type%" == "copy" (
    if not exist "%OPEN_SRC_3RD_DIR%" (
        echo "Failed to Copy the open source package,the path is not exist:%OPEN_SRC_3RD_DIR%."
        exit /b 1
    )
    if not exist "%AGENT_OPEN_SRC_PATH%" (
        md  "%AGENT_OPEN_SRC_PATH%"
    )
    xcopy /y /e "%OPEN_SRC_3RD_DIR%" "%AGENT_OPEN_SRC_PATH%"
    echo "The open-source package is copied successfully."
) else (
    artget pull -d %CurBatPath%../../ci/LCRP/conf/dependency_opensrc_win.xml -p "{'OPENSOURCE_BRANCH': '%OPENSOURCE_BRANCH%', 'SYS_NAME':'%SYS_NAME%', 'SYS_ARCH':'%SYS_ARCH%', 'Version':'%Version%'}" -user %cmc_user% -pwd %cmc_pwd% -ap %CurBatPath%../../open_src
    if NOT %errorlevel% EQU 0 (
        echo "Failed to download the open source package."
        exit /b 1
    )
    echo "The open-source package is downloaded successfully."
)

call :Retry_unzip
if NOT %errorlevel% EQU 0 (
    endlocal
    exit /b 1
)
endlocal
exit /b 0


:Retry_unzip
    for %%a in (1 2 3) do (
        echo "Check whether the open source package is decompressed, try %%a/3."
        cd "%CurBatPath%../../open_src"
        7z.exe x -y OceanProtect_X8000_%Version%_Opensrc_3rd_SDK_%SYS_NAME%_%SYS_ARCH%.zip
        cd "%CurBatPath%"
        call :Check_open_src_packages
        if !errorlevel! EQU 0 (
            echo "The open source package is decompressed successfully."
            exit /b 0
        )
    )
    echo "The open source package is decompressed failed."
    exit /b 1

:Check_open_src_packages
    echo "Checking the open source package. Please wait..."

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_JSONCPP_DIR%" (
        if not exist "%AGENT_OPEN_SRC_PATH%\jsoncpp_00.11.0" (
            echo "OpenSrc package[jsoncpp_00.11.0] is not exist."
            exit /b 1
        )
        xcopy /y /e "%AGENT_OPEN_SRC_PATH%\jsoncpp_00.11.0" "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_JSONCPP_DIR%\"
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%" (
        if not exist "%AGENT_OPEN_SRC_PATH%\fcgi2" (
            echo "OpenSrc package[%AGENT_OPEN_SRC_PATH%\fcgi2] is not exist."
            exit /b 1
        )
        xcopy /y /e "%AGENT_OPEN_SRC_PATH%\fcgi2" "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_FCGI_DIR%\"
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_NGINX_DIR%" (
        if not exist "%AGENT_OPEN_SRC_PATH%\nginx" (
            echo "OpenSrc package[nginx] is not exist."
            exit /b 1
        )
        xcopy /y /e "%AGENT_OPEN_SRC_PATH%\nginx" "%AGENT_OPEN_SRC_PATH%\nginx_back\"
        move /y "%AGENT_OPEN_SRC_PATH%\nginx" "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_NGINX_DIR%"
        md  "%AGENT_OPEN_SRC_PATH%\nginx"
        xcopy /y /e "%AGENT_OPEN_SRC_PATH%\objs_ngx" "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_NGINX_DIR%\"
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_SQLITE_DIR%" (
        echo "OpenSrc package[%OPEN_SRC_SQLITE_DIR%] is not exist."
        exit /b 1
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_SNMP_DIR%" (
        if not exist "%AGENT_OPEN_SRC_PATH%\SNMP" (
            echo "OpenSrc package[SNMP] is not exist."
            exit /b 1
        )
        xcopy /y /e "%AGENT_OPEN_SRC_PATH%\SNMP" "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_SNMP_DIR%\"
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_TINYXML_DIR%" (
        if not exist "%AGENT_OPEN_SRC_PATH%\tinyxml2" (
            echo "OpenSrc package[tinyxml2] is not exist."
            exit /b 1
        )
        xcopy /y /e "%AGENT_OPEN_SRC_PATH%\tinyxml2" "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_TINYXML_DIR%\"
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_BOOST_DIR%" (
        echo "OpenSrc package[%OPEN_SRC_BOOST_DIR%] is not exist."
        exit /b 1
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_THRIFT_DIR%" (
        echo "OpenSrc package[%OPEN_SRC_THRIFT_DIR%] is not exist."
        exit /b 1
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_LIBEVENT_DIR%" (
        echo "OpenSrc package[%OPEN_SRC_LIBEVENT_DIR%] is not exist."
        exit /b 1
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_ZLIB_DIR%" (
        echo "OpenSrc package[%OPEN_SRC_ZLIB_DIR%] is not exist."
        exit /b 1
    )

    if not exist "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_OPENSSL_DIR%" (
        echo "OpenSrc package[%OPEN_SRC_OPENSSL_DIR%] is not exist."
        exit /b 1
    ) else (
        if not exist "%AGENT_OPEN_SRC_PATH%\openssl_temp" (
            xcopy /y /e "%AGENT_OPEN_SRC_PATH%\%OPEN_SRC_OPENSSL_DIR%" "%AGENT_OPEN_SRC_PATH%\openssl_temp\"
        )
    )

    if not exist %AGENT_OPEN_SRC_PATH%\%OPEN_SRC_7ZIP_DIR% (
        echo "OpenSrc package[%OPEN_SRC_7ZIP_DIR%] is not exist."
        exit /b 1
    )

    exit /b 0