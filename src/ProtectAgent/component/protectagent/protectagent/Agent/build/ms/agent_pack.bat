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
rem 对工程应该首先编译，然后拷贝必要的文件，然后做安装包放置到Agent相关out目录下

rem 设置变量
@echo off

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set AGENTPATH=%CurBatPath%..\..\
SET INSTALLSHIELD_PACKAGE_DIR=%AGENTPATH%build\ms\package
set BUILD_LOG="%CurBatPath%"build.log

set VERSION_PARAM="mp_string AGENT_PACKAGE_VERSION"
set VERSION=
call :FindAgentVERSION VERSION
echo VERSION is %VERSION%

set VERSION_PARAM="mp_string AGENT_VERSION"
set AGENT_VERSION=
call :FindAgentVERSION AGENT_VERSION
echo AGENT_VERSION is %AGENT_VERSION%

set VERSION_PARAM="mp_string AGENT_BUILD_NUM"
set AGENT_BUILD_NUM=
call :FindAgentVERSION AGENT_BUILD_NUM
echo AGENT_BUILD_NUM is %AGENT_BUILD_NUM%

rem 获得系统的日期
rem 判断系统date变量格式
for /f "tokens=1" %%a in ('date /T') do (
    echo %%a| findstr "^[\/0-9]*$" >nul && (
        set YEAR=%date:~0,4%
        set MOUNT=%date:~5,2%
        set DAY=%date:~8,2%
    ) || (
        set YEAR=%date:~-4%
        set MOUNT=%date:~-10,-8%
        set DAY=%date:~-7,-5%
    )
)
rem 获得系统的时间
rem 判断系统time变量是否需要补0
if "%time:~0,1%" EQU " " (
    set HOUR=0%time:~1,1%
) else (
    set HOUR=%time:~0,2%
)
set MINUTE=%time:~3,2%
set SECOND=%time:~6,2%

set AGENT_UPDATE_VERSION=%YEAR%%MOUNT%%DAY%%HOUR%%MINUTE%%SECOND%
echo AGENT_UPDATE_VERSION is %AGENT_UPDATE_VERSION%

set AGENT_PRODUCT_NAME=OceanStor A8000
if "eBackup" == "%~1" (
    set AGENT_PRODUCT_NAME=Cloud Server Backup
)
set BIN_NAME_BASE=protectclient-Windows-x64
rem===================================================

echo %PATH% >> %BUILD_LOG%

rem unzip the open_src files
call %CurBatPath%agent_make_opensrc.bat

echo 1 is %1
if "%1" EQU "Clean" (GOTO Clean)

rem call 64 compile
call %CurBatPath%agent_make.bat compile64agent

if not "%errorlevel%" == "0" (
    echo "Exec %CurBatPath%agent_make.bat compile64agent fail."
    exit /b 1
)

SET PACKAGE_FILE_DIR=%AGENTPATH%pkg
SET AGENT_SDK_DIR=%AGENTPATH%plugin_sdk
SET SOURCE_FILE_DIR=%AGENTPATH%bin

IF EXIST %PACKAGE_FILE_DIR%                     RMDIR  /S/Q  %PACKAGE_FILE_DIR%
MKDIR  %PACKAGE_FILE_DIR%
MKDIR  %PACKAGE_FILE_DIR%\bin
MKDIR  %PACKAGE_FILE_DIR%\bin\plugins
MKDIR  %PACKAGE_FILE_DIR%\bin\thirdparty
MKDIR  %PACKAGE_FILE_DIR%\bin\thirdparty\sample
MKDIR  %PACKAGE_FILE_DIR%\bin\thirdparty\rd_user
MKDIR  %PACKAGE_FILE_DIR%\conf
MKDIR  %PACKAGE_FILE_DIR%\db
MKDIR  %PACKAGE_FILE_DIR%\db\upgrade
MKDIR  %PACKAGE_FILE_DIR%\tmp
MKDIR  %PACKAGE_FILE_DIR%\log
MKDIR  %PACKAGE_FILE_DIR%\Tool
MKDIR  %PACKAGE_FILE_DIR%\Tool\curl
MKDIR  %PACKAGE_FILE_DIR%\Tool\sqlite

IF EXIST %AGENT_SDK_DIR%                     RMDIR  /S/Q  %AGENT_SDK_DIR%
MKDIR  %AGENT_SDK_DIR%
MKDIR  %AGENT_SDK_DIR%\lib
MKDIR  %AGENT_SDK_DIR%\include
MKDIR  %AGENT_SDK_DIR%\include\common
MKDIR  %AGENT_SDK_DIR%\include\securec
MKDIR  %AGENT_SDK_DIR%\include\message
MKDIR  %AGENT_SDK_DIR%\include\message\archivestream

rem==================1. set conf files begin======================
rem 生成version文件
echo %AGENT_VERSION%>%PACKAGE_FILE_DIR%\conf\version
echo %AGENT_BUILD_NUM%>>%PACKAGE_FILE_DIR%\conf\version
echo %AGENT_UPDATE_VERSION%>>%PACKAGE_FILE_DIR%\conf\version
rem==================1. set conf files end========================

rem==================2. copy bin file begin=======================

IF NOT EXIST %SOURCE_FILE_DIR%\rdagent.exe                           call :FileNotFound rdagent.exe
COPY /Y %SOURCE_FILE_DIR%\rdagent.exe                                %PACKAGE_FILE_DIR%\bin\rdagent.exe

IF NOT EXIST %SOURCE_FILE_DIR%\crypto.exe                            call :FileNotFound crypto.exe
COPY /Y %SOURCE_FILE_DIR%\crypto.exe                                 %PACKAGE_FILE_DIR%\bin\crypto.exe

IF NOT EXIST %SOURCE_FILE_DIR%\scriptsign.exe                        call :FileNotFound scriptsign.exe
COPY /Y %SOURCE_FILE_DIR%\scriptsign.exe                             %PACKAGE_FILE_DIR%\bin\scriptsign.exe

IF NOT EXIST %SOURCE_FILE_DIR%\datamigration.exe                     call :FileNotFound datamigration.exe
COPY /Y %SOURCE_FILE_DIR%\datamigration.exe                          %PACKAGE_FILE_DIR%\bin\datamigration.exe

IF NOT EXIST %SOURCE_FILE_DIR%\monitor.exe                           call :FileNotFound monitor.exe
COPY /Y %SOURCE_FILE_DIR%\monitor.exe                                %PACKAGE_FILE_DIR%\bin\monitor.exe

IF NOT EXIST %SOURCE_FILE_DIR%\winservice.exe                        call :FileNotFound winservice.exe
COPY /Y %SOURCE_FILE_DIR%\winservice.exe                             %PACKAGE_FILE_DIR%\bin\winservice.exe

IF NOT EXIST %SOURCE_FILE_DIR%\xmlcfg.exe                            call :FileNotFound xmlcfg.exe
COPY /Y %SOURCE_FILE_DIR%\xmlcfg.exe                                 %PACKAGE_FILE_DIR%\bin\xmlcfg.exe

IF NOT EXIST %SOURCE_FILE_DIR%\agentcli.exe                          call :FileNotFound agentcli.exe
COPY /Y %SOURCE_FILE_DIR%\agentcli.exe                               %PACKAGE_FILE_DIR%\bin\agentcli.exe

IF NOT EXIST %SOURCE_FILE_DIR%\getinput.exe                          call :FileNotFound getinput.exe
COPY /Y %SOURCE_FILE_DIR%\getinput.exe                               %PACKAGE_FILE_DIR%\bin\getinput.exe


IF NOT EXIST %SOURCE_FILE_DIR%\libcommon.dll                         call :FileNotFound libcommon.dll
COPY /Y %SOURCE_FILE_DIR%\libcommon.dll                              %PACKAGE_FILE_DIR%\bin\libcommon.dll

IF NOT EXIST %SOURCE_FILE_DIR%\libsecurecom.dll                         call :FileNotFound libsecurecom.dll
COPY /Y %SOURCE_FILE_DIR%\libsecurecom.dll                              %PACKAGE_FILE_DIR%\bin\libsecurecom.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libappprotect.dll                call :FileNotFound libappprotect.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libappprotect.dll                     %PACKAGE_FILE_DIR%\bin\plugins\libappprotect-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libhost.dll                 call :FileNotFound libhost.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libhost.dll                      %PACKAGE_FILE_DIR%\bin\plugins\libhost-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\bat\dist\CreateDataturbolink.exe                 call :FileNotFound CreateDataturbolink
COPY /Y %SOURCE_FILE_DIR%\bat\dist\CreateDataturbolink.exe                %PACKAGE_FILE_DIR%\bin\CreateDataturbolink.exe

IF NOT EXIST %SOURCE_FILE_DIR%\bat\dist\DataturboUmount.exe                 call :FileNotFound DataturboUmount
COPY /Y %SOURCE_FILE_DIR%\bat\dist\DataturboUmount.exe                %PACKAGE_FILE_DIR%\bin\DataturboUmount.exe

COPY /Y %SOURCE_FILE_DIR%\bat\thirdparty\sample\*                    %PACKAGE_FILE_DIR%\bin\thirdparty\sample\
COPY /Y %SOURCE_FILE_DIR%\CustomScripts\Windows\*                    %PACKAGE_FILE_DIR%\bin\thirdparty\
COPY /Y %SOURCE_FILE_DIR%\bat\thirdparty\rd_user\*                    %PACKAGE_FILE_DIR%\bin\thirdparty\rd_user\

COPY /Y %AGENTPATH%third_party_groupware\msvc80\lib\*               %PACKAGE_FILE_DIR%\bin\

COPY /Y %AGENTPATH%open_src\openssl\out32\bin\openssl.exe               %PACKAGE_FILE_DIR%\bin\
COPY /Y %AGENTPATH%open_src\openssl\apps\openssl.cnf                %PACKAGE_FILE_DIR%\conf\

echo  %DATE% %TIME% %1 copy bin files >> %BUILD_LOG%

rem==================2. copy bin file end=======================

rem==================3. copy  nginx file begin=================
MKDIR  %PACKAGE_FILE_DIR%\nginx
MKDIR  %PACKAGE_FILE_DIR%\nginx\conf
MKDIR  %PACKAGE_FILE_DIR%\nginx\logs

IF NOT EXIST %AGENTPATH%open_src\nginx_tmp\objs\nginx.exe      call :FileNotFound nginx.exe
COPY /Y %AGENTPATH%open_src\nginx_tmp\objs\nginx.exe           %PACKAGE_FILE_DIR%\nginx\rdnginx.exe

COPY /Y %AGENTPATH%conf\backup\nginx.conf                          %PACKAGE_FILE_DIR%\nginx\conf\nginx.conf
COPY /Y %AGENTPATH%conf\fastcgi_params                          %PACKAGE_FILE_DIR%\nginx\conf\fastcgi_params
COPY /Y %AGENTPATH%conf\server.crt                          %PACKAGE_FILE_DIR%\nginx\conf\server.pem
COPY /Y %AGENTPATH%conf\server.key                          %PACKAGE_FILE_DIR%\nginx\conf\server.key
COPY /Y %AGENTPATH%conf\bcmagentca.crt                          %PACKAGE_FILE_DIR%\nginx\conf\pmca.pem
COPY /Y %AGENTPATH%conf\kmc_store_bak.txt                          %PACKAGE_FILE_DIR%\nginx\conf\kmc_store_bak.txt

echo  %DATE% %TIME% %1 copy nginx files >> %BUILD_LOG%
rem==================3. copy  nginx file end==================

rem==================4. copy bat file begin====================
COPY /Y %SOURCE_FILE_DIR%\bat\*.bat %PACKAGE_FILE_DIR%\bin\
COPY /Y %SOURCE_FILE_DIR%\bat\*.cmd %PACKAGE_FILE_DIR%\bin\
COPY /Y %SOURCE_FILE_DIR%\bat\*.ps1 %PACKAGE_FILE_DIR%\bin\
COPY /Y %SOURCE_FILE_DIR%\bat\*.vbs %PACKAGE_FILE_DIR%\bin\
COPY /Y %SOURCE_FILE_DIR%\install\win32\ProtectClient-E\*.bat %PACKAGE_FILE_DIR%\bin\

echo  %DATE% %TIME% %1 copy bat script files >> %BUILD_LOG%
rem==================4. copy bat file end======================

rem==================5. copy conf file begin===================
IF EXIST %AGENTPATH%conf\kmc ( COPY /Y %AGENTPATH%conf\kmc\* %PACKAGE_FILE_DIR%\conf\kmc\ )
COPY /Y %AGENTPATH%conf\backup\*.xml                                %PACKAGE_FILE_DIR%\conf\
COPY /Y %AGENTPATH%conf\backup\version                              %PACKAGE_FILE_DIR%\conf\
COPY /Y %AGENTPATH%conf\kmc_config.txt                       %PACKAGE_FILE_DIR%\conf\
COPY /Y %AGENTPATH%conf\kmc_store.txt                        %PACKAGE_FILE_DIR%\conf\

echo  %DATE% %TIME% %1 copy xml files in conf dir >> %BUILD_LOG%
rem==================5. copy conf file end=====================

rem==================6. copy db file begin=====================
COPY /Y %AGENTPATH%selfdevelop\*.db                          %PACKAGE_FILE_DIR%\db\
COPY /Y %SOURCE_FILE_DIR%\install\win32\ProtectClient-E\upgrade\* %PACKAGE_FILE_DIR%\db\upgrade

echo  %DATE% %TIME% %1 copy db file in db dir >> %BUILD_LOG%
rem==================6. copy db file end=======================

rem==================7. gen sign file begin=====================
"%PACKAGE_FILE_DIR%\bin\scriptsign.exe"
DEL /F "%PACKAGE_FILE_DIR%\bin\scriptsign.exe"
DEL /F "%PACKAGE_FILE_DIR%\log\scriptsign.log"

echo  %DATE% %TIME% %1 gen script sign file in conf dir >> %BUILD_LOG%
rem==================7. gen sign file end=======================

rem==================8. copy Open Source Software Notice.doc============
COPY /Y "%AGENTPATH%build\copyRight\Open Source Software Notice.doc"  %PACKAGE_FILE_DIR%\
rem==================8. copy Open Source Software Notice.doc end========

rem==================9. copy 7zip begin========================
COPY /Y %AGENTPATH%third_party_groupware\7Zip\*   %PACKAGE_FILE_DIR%\bin\
COPY /Y %AGENTPATH%open_src\sqlite\sqlite3.exe   %PACKAGE_FILE_DIR%\Tool\sqlite\sqlite3.exe
COPY /Y %AGENTPATH%open_src\curl\builds\bin\bin\*   %PACKAGE_FILE_DIR%\Tool\curl\
rem==================9. copy 7zip end==========================

rem==================10. initialize agent config xml begin======================
set NGINX_SSL_KEY_PASSWD=000000010000000100000000000000050000000100000001b343ffca851b6d034c6bb7d445fd4af708a3ee07194e9743b540044239695b2b000000200000000000000000078c3a332e83382aae523e3a0c0b69bab0624250a72a5cb1655cab72e2f607860000000100000000000008040000000100000001410d4a21dd845af48ffb5e0ef920aece00000000000000001f63687345f329caacd70fc94b202ee87e4d5bd85385d0e971cafe26b31f841b
call %PACKAGE_FILE_DIR%\bin\xmlcfg.exe write Monitor nginx ssl_key_password "%NGINX_SSL_KEY_PASSWD%"

set SNMP_PRIVATE_PASSWD=00000001000000010000000000000005000000010000000153d515e41001697d5474c6f329a671cb3ba511f45c21ec57572163012c0c354e0000002000000000000000006b181ea5dc07d3f3646c8d17bddbca2966483054258e4b33d07b77af771e145e0000000100000000000008040000000100000001e5aa69c27edd0c2181b9898d1f3de54c00000000000000001ef5368807eda14b417b61330ded694e63798d17e9de90ab5a1374ecabce8f89
call %PACKAGE_FILE_DIR%\bin\xmlcfg.exe write SNMP private_password "%SNMP_PRIVATE_PASSWD%"

set SNMP_AUTH_PASSWD=000000010000000100000000000000050000000100000001c1b831a0fbb4e9f7bcdf5f9f4903357dfd6306be5b3c74eba3ac2d5081a2b0d2000000200000000000000000da7037e7e90938720c84c633cb6e1d231c47907a657f5c1f646755fa695310ed000000010000000000000804000000010000000198e0022f6e462a60bac5c6f5ef9d1e040000000000000000a17ab32d8a80c0e64782ed86bb9039fcac47d166699e01e7a643f910eb32ae77
call %PACKAGE_FILE_DIR%\bin\xmlcfg.exe write SNMP auth_password "%SNMP_AUTH_PASSWD%"
del /f /q %PACKAGE_FILE_DIR%\log\*
rem==================10. initialize agent config xml end========================

rem==================11. copy agent sdk begin======================
COPY /Y "%SOURCE_FILE_DIR%\agentsdk.dll"  %AGENT_SDK_DIR%\lib
COPY /Y "%SOURCE_FILE_DIR%\agentsdk.lib"  %AGENT_SDK_DIR%\lib
COPY /Y "%AGENTPATH%\src\inc\pluginfx\ExternalPluginSDK.h"  %AGENT_SDK_DIR%\include

COPY /Y "%AGENTPATH%\src\inc\common\Defines.h"  %AGENT_SDK_DIR%\include\common
COPY /Y "%AGENTPATH%\src\inc\common\Types.h"  %AGENT_SDK_DIR%\include\common
COPY /Y "%AGENTPATH%\platform\securec\include\securec.h"  %AGENT_SDK_DIR%\include\securec
COPY /Y "%AGENTPATH%\platform\securec\include\securectype.h"  %AGENT_SDK_DIR%\include\securec
COPY /Y "%AGENTPATH%\src\inc\message\archivestream\ArchiveStreamService.h"  %AGENT_SDK_DIR%\include\message\archivestream
rem==================11. copy agent sdk end========================

rem==================12. zip Agent begin========================
"%AGENTPATH%\third_party_groupware\7Zip\7z.exe" a "%INSTALLSHIELD_PACKAGE_DIR%\protectclient-Windows-x64.zip" "%PACKAGE_FILE_DIR%\*"
rem "%AGENTPATH%\third_party_groupware\7Zip\7z.exe" a "%INSTALLSHIELD_PACKAGE_DIR%\plugin_sdk.zip" "%AGENT_SDK_DIR%\include" "%AGENT_SDK_DIR%\lib"

echo  %DATE% %TIME% %1 zip Agent >> %BUILD_LOG%
rem==================12. zip Agent end==========================

echo Release Success. 
exit /b 0

rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
rem
:FindAgentVERSION

    rem Query the UltraAgent Version 
    set %1=n/a
    set fileWithInfo=%AGENTPATH%src\inc\common\AppVersion.h
    if not exist %fileWithInfo% (
        goto :eof
    )
    for /f tokens^=2*^ delims^=^" %%i in ('type %fileWithInfo% ^| findstr /C:%VERSION_PARAM%') do (
        rem Because the content of %%1 is a sentence, so we use "" to make it as a whole
        rem the side effect is the %1 become as "the content of %%1", in fact, "" is redundant
        set %1=%%i
    )
goto :eof

:FileNotFound
echo  %DATE% %TIME% %1 Not Found >> %BUILD_LOG%
goto Exit 

:Clean
call %AGENTPATH%build\ms\BCM\agent_make.bat Clean64Openssl
goto Exit

:Exit
exit /b 1
            
