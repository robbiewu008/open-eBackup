@echo off
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
setlocal EnableDelayedExpansion

set CURRENTPATH=%~dp0
set AGENT_ROOT=%CURRENTPATH%..\
set AGENT_CONF_PATH=%AGENT_ROOT%conf\
set MAXLOGSIZE=52428800
set LOGFILE_SUFFIX=zip
set TEMPLOGNAME=%~nx1
set LOGFILENAME=%AGENT_ROOT%log\%TEMPLOGNAME%
set BACKLOGNAME="%LOGFILENAME%.%BACKLOGCOUNT%"

for /f "delims=" %%i in ("%LOGFILENAME%") do (
    set filesize=%%~zi
) 

for /f tokens^=1-3^ delims^=^" %%i in ('findstr "log_count" "%AGENT_CONF_PATH%agent_cfg.xml"') do (
    set  BACKLOGCOUNT=%%j
)

set BACKLOGNAME="%LOGFILENAME%.%BACKLOGCOUNT%.%LOGFILE_SUFFIX%"
if !filesize! gtr %MAXLOGSIZE% (
    if exist !BACKLOGNAME! (del /f /q !BACKLOGNAME!)
    set /a NUMBER=%BACKLOGCOUNT%-1
    :backlog
    if !NUMBER! GEQ 0 (
        if !NUMBER! EQU 0 (
            set BACKLOGNAME="%LOGFILENAME%.%LOGFILE_SUFFIX%"
            call "%CURRENTPATH%7z.exe" a -y -tzip %BACKLOGNAME% "%LOGFILENAME%" -mx=9 > nul
            del /f /q  "%LOGFILENAME%"
        ) else (
            set BACKLOGNAME="%LOGFILENAME%.!NUMBER!.%LOGFILE_SUFFIX%"
        )
        if exist !BACKLOGNAME! (
            set /a NUMBER_TEMP=!NUMBER!+1
            set DESTLOGNAME="%TEMPLOGNAME%.!NUMBER_TEMP!.%LOGFILE_SUFFIX%"
            REN !BACKLOGNAME! !DESTLOGNAME!
        )
        set /a NUMBER=!NUMBER!-1
        goto :backlog
    )
    cd ./>"%LOGFILENAME%"
)

rem set file access
echo>nul 2>nul Y | Cacls "%LOGFILENAME%" /E /R Users > nul

endlocal
