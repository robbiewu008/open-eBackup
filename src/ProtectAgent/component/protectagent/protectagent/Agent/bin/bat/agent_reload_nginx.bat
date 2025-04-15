@echo off
:: 
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
::
setlocal EnableDelayedExpansion

set RETRY_COUNT=1
set SC_RETRY_COUNT=1

sc stop rdnginx

:startagentservice
    sc start rdnginx
    
    :waitstart
        call :Winsleep 3
        
        set SERVICE_CHECK=
        for /f "delims=" %%i in ('2^>nul sc query rdnginx ^| find "RUNNING"') do (set SERVICE_CHECK=%%i)
        
        if not "!SERVICE_CHECK!" == "" (
            exit /b 0
        ) else (
            rem retry 5 times 
            if !RETRY_COUNT! LEQ 5 (
                set /a RETRY_COUNT+=1
                goto :waitstart
            )

            if !SC_RETRY_COUNT! LEQ 3 (
                set /a SC_RETRY_COUNT+=1
                set RETRY_COUNT=1
                goto :startagentservice
            )

            exit /b 1           
        )

:Winsleep
    ping 127.0.0.1 -n %~1 > nul
    exit /b 0