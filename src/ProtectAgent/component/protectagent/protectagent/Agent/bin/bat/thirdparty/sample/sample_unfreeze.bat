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
rem The parameters %1 is passed by Agent. So Please does not change following script.
set CURR_PATH=%~dp0
set RSTFILE="%CURR_PATH%"\..\..\tmp\RST%1.txt
set LOGFILE="%CURR_PATH%"\..\..\log\sample.log
set PARAM_FILE="%CURR_PATH%"\..\..\tmp\input_tmp%1

set UNFREEZE_FILE="%CURR_PATH%\..\..\tmp\unfreezedb%1.sql"
set UNFREEZERST_FILE="%CURR_PATH%\..\..\tmp\unfreezedbRST%1.txt"

rem The related business script code need to be here.
rem ########Begin########
echo "Begin to do unfreeze db." >> %LOGFILE%
if exist %PARAM_FILE% (del /f /q %PARAM_FILE%)

rem if the operation is successed, need to write blank string into the result file %RSTFILE%. 
rem Otherwise please write material error infomation into the result file %$RSTFILE%.
rem For example,
set ORACLE_SID=dbFS
echo alter database end backup; > %UNFREEZE_FILE%
echo exit; >> %UNFREEZE_FILE%
sqlplus / as sysdba @%UNFREEZE_FILE% > %UNFREEZERST_FILE%
if not exist %UNFREEZERST_FILE% (
    echo "Exec SQL failed." >> %LOGFILE%
    if exist %UNFREEZE_FILE% (del /f /q %UNFREEZE_FILE%)
    if exist %UNFREEZERST_FILE% (del /f /q %UNFREEZERST_FILE%)
    endlocal
    exit 1
) else (
    type %UNFREEZERST_FILE% | findstr "ERROR" >nul
    if !errorlevel! EQU 0 (
        echo "=====================Database execsql failed=====================" >> %LOGFILE%
        type %UNFREEZERST_FILE% >> %LOGFILE%
        echo "=====================Database execsql failed=====================" >> %LOGFILE%
        if exist %UNFREEZE_FILE% (del /f /q %UNFREEZE_FILE%)
        if exist %UNFREEZERST_FILE% (del /f /q %UNFREEZERST_FILE%)
        endlocal
        exit 1
    )
    if exist %UNFREEZE_FILE% (del /f /q %UNFREEZE_FILE%)
    if exist %UNFREEZERST_FILE% (del /f /q %UNFREEZERST_FILE%)
)

echo "Finish doing unfreeze db." >> %LOGFILE%
endlocal
exit 0
rem ########End#######
