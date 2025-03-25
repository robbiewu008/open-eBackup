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

cd /d %~dp0
set CURRENT_PATH=%~dp0
set WIN_SYSTEM_DISK=%WINDIR:~0,1%
set HOSTSN_FILE=C:\Users\Default\HostSN
if not "%WIN_SYSTEM_DISK%" == "" (
    set HOSTSN_FILE=%WIN_SYSTEM_DISK%:\Users\Default\HostSN
)
set ipv4=
set ipv6=
set hostsn=


rem ipv4
for /f "delims=: tokens=2" %%a in ('ipconfig ^|find /i "ipv4"') do (
    call set ipv4=%%ipv4%%,%%a
)
if "!ipv4!" NEQ "" (
    set ipv4=!ipv4:~1!
)
if "!ipv4!" NEQ "" (
    set ipv4=!ipv4: =!
)


rem ipv6
for /f "tokens=1 delims=%%" %%a in ('ipconfig ^|find /i "ipv6"') do (
    for /f "tokens=10" %%b in ("%%a") do (
        call set ipv6=%%ipv6%%,%%b
    )
)
if "!ipv6!" NEQ "" (
    set ipv6=!ipv6:~1!
)
if "!ipv6!" NEQ "" (
    set ipv6=!ipv6: =!
)


rem uuid
if exist !HOSTSN_FILE! (
    set /P hostsn=<!HOSTSN_FILE!
)


rem write to file
echo ipv4=!ipv4!
echo ipv6=!ipv6!
echo uuid=!hostsn!

echo ipv4=!ipv4!>!CURRENT_PATH!\check_result.txt
echo ipv6=!ipv6!>>!CURRENT_PATH!\check_result.txt
echo uuid=!hostsn!>>!CURRENT_PATH!\check_result.txt

endlocal