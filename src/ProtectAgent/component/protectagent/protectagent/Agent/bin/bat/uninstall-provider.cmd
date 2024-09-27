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

@rem @echo off
@setlocal

@set FOUND_FILES=0
@set CMD_DIR=%~dp0
@set DLL_DIR=%CMD_DIR%
@set VBS_DIR=%CMD_DIR%
@call :checkfiles
@if %FOUND_FILES% EQU 0 (goto :missingfiles) else (goto :startuninstall)


:startuninstall

net stop vds
net stop vss
net stop swprv

cscript "%VBS_DIR%\register_app.vbs" -unregister "RdProvider"
regsvr32 /s /u "%DLL_DIR%\rdvss.dll"

echo.
goto :EOF


:checkfiles 

@if not exist "%DLL_DIR%\rdvss.dll"             goto :EOF
@if not exist "%VBS_DIR%\register_app.vbs"      goto :EOF
@set FOUND_FILES=1
@goto :EOF


:missingfiles

@echo.
@echo One or more important files are missing.
@echo.
@echo   RDProvider.dll
@echo   register_app.vbs
@echo   install-provider.cmd
@echo   uninstall-provider.cmd
@echo.
@goto :EOF
