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

@echo ons
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set PACK_LOG="%CurBatPath%CI_PACK.log"
set BASE_PATH=%CurBatPath%..\
echo BASE_PATH is %BASE_PATH% >> %PACK_LOG%
set FRAMEWORK_PATH=%BASE_PATH%..\..\framework
set OUTPUT_PKG_PATH=%FRAMEWORK_PATH%\output_pkg

call :main
endlocal
goto :eof

:execute_build_script
    call build_file.bat
    if %errorlevel% NEQ 0 (
        echo Failed to execute the build script! >> %PACK_LOG%
        exit 1
    )
goto :eof

:create_dir
    ::create bin/applications path
    if not exist %OUTPUT_PKG_PATH%\bin mkdir %OUTPUT_PKG_PATH%\bin

    ::create conf path
    if not exist %OUTPUT_PKG_PATH%\conf mkdir %OUTPUT_PKG_PATH%\conf

    ::create service path
    if not exist %OUTPUT_PKG_PATH%\install mkdir %OUTPUT_PKG_PATH%\install
goto :eof


:copy_file
    ::copy plugin_attribute json file
    if not exist %BASE_PATH%\conf\plugin_attribute_*.json (
        echo The plugin attribute json file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\conf\plugin_attribute_*.json %OUTPUT_PKG_PATH%\conf /e/y

    ::copy dll file
    if not exist %BASE_PATH%\lib\*.dll (
        echo The FilePlugin exe file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\lib\*.dll %OUTPUT_PKG_PATH%\bin /e/y

    ::copy other conf file
    xcopy %BASE_PATH%\conf\*.conf %OUTPUT_PKG_PATH%\conf /e/y

    ::copy hcpconf.ini
    if not exist %BASE_PATH%\conf\hcpconf.ini (
        echo The hcpconf.ini file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\conf\hcpconf.ini %OUTPUT_PKG_PATH%\conf /e/y

    ::copy param_check.xml
    if exist %BASE_PATH%\conf\param_check.xml (
        xcopy %BASE_PATH%\conf\param_check.xml %OUTPUT_PKG_PATH%\conf /e/y
    )

    ::copy install.bat
    if not exist %BASE_PATH%\build\install\install.bat (
        echo The install file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\build\install\install.bat %OUTPUT_PKG_PATH%\install /e/y
    
    ::copy start.bat
    if not exist %BASE_PATH%\build\install\start.bat (
        echo The start file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\build\install\start.bat %OUTPUT_PKG_PATH%\install /e/y

goto :eof


:execute_pack_script
    call %FRAMEWORK_PATH%\build\pack.bat
    if %errorlevel% NEQ 0 (
        echo Failed to execute framework pack script! >> %PACK_LOG%
        exit 1
    )
goto :eof

:main
    echo "#######################################################################################################"
    echo Start pack FilePlugin.
    echo Start pack FilePlugin. >> %PACK_LOG%
    echo "#######################################################################################################"

    call :execute_build_script
    
    call :create_dir

    call :copy_file

    call :execute_pack_script

    echo "#######################################################################################################"
    echo FilePlugin pack success.
    echo FilePlugin pack success. >> %PACK_LOG%
    echo "#######################################################################################################"
goto :eof