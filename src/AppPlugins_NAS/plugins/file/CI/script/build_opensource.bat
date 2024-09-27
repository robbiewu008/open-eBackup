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
set BASE_PATH=%CurBatPath%..\..\
echo BASE_PATH is %BASE_PATH% >> %PACK_LOG%
set FRAMEWORK_PATH=%BASE_PATH%..\..\framework
set OUTPUT_PKG_PATH=%FRAMEWORK_PATH%\output_pkg
set PLUGIN_PATH=%CurBatPath%..\..\..\..
set open_source_obligation_path=%BASE_PATH%..\..\..\open-source-obligation

call :main
endlocal
goto :eof

:execute_build_script

    xcopy /y /e %open_source_obligation_path%\Windows_Compile\  %PLUGIN_PATH%\Windows_Compile\
    echo d | xcopy /y /e %open_source_obligation_path%\Windows_Compile\  %PLUGIN_PATH%\Windows_Compile\

    call pack_openrepo.bat
    if %errorlevel% NEQ 0 (
       echo Failed to execute the pack_openrepo script! >> %PACK_LOG%
       exit 1
    )
goto :eof

:main
    echo "#######################################################################################################"
    echo Start pack FilePlugin.
    echo Start pack FilePlugin. >> %PACK_LOG%
    echo "#######################################################################################################"

    call :execute_build_script

    echo "#######################################################################################################"
    echo FilePlugin pack success.
    echo FilePlugin pack success. >> %PACK_LOG%
    echo "#######################################################################################################"
goto :eof