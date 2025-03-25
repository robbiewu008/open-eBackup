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
rem @dest:   application agent for oracle
rem @date:   2020-07-16
rem @authr:  
rem @modify:

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERR_FILE_IS_EMPTY=8

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=preparenasmedia.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set DISKPART_CMD_FILE="%AGENT_TMP_PATH%diskCmd%PID%"
rem #############################################################

set INPUTINFO=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTINFO=%%a
    )
)
rem call :DeleteFile %PARAM_FILE%
if "!INPUTINFO!" == "" (
    call :Log "INPUTINFO is null."
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :end
)

call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "dataShareMountPath" dataPath
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "logShareMountPath" logPath
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "storageIp" storageIP
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "AppName" dbName
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "authUser" authUser
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "dataPath" dataMount
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "logPath" logMount
call :Log "dataPath=%dataPath%;logPath=%logPath%;storageIP=%storageIP%;dbName=%dbName%;authUser=%authUser%;dataMount=%dataMount%;logMount=%logMount%"

set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
if "!dataPath!" == "" (
    call :Log "dataPath is null."
    goto :end
)

if "!logPath!" == "" (
    call :Log "logPath is null."
    goto :end
)

if "!storageIP!" == "" (
    call :Log "storageIP is null."
    goto :end
)

if "!dbName!" == "" (
    call :Log "dbName is null."
    goto :end
)

call :Log "Begin to prepare nas backup medium."
set ERR_CODE=0

set dataDriver=
set logDriver=
rem get mapped path
for /f "tokens=2 delims= " %%a in ('net use | findstr "%storageIP%\\%dataPath%"') do (
    if not "%%a" == "" (
        set dataDriver=%%a
    )
)

for /f "tokens=2 delims= " %%a in ('net use | findstr "%storageIP%\\%logPath%"') do (
    if not "%%a" == "" (
        set logDriver=%%a
    )
)

if "!dataDriver!" == "" (
    rem get unused disk driver
    set str=z y x w v u t s r q p o n m l k j i h g f e d c
    for %%i in (%str%) do (
        if not exist %%i: (
            if not "!dataDriver!" == "" (
                dataDriver=%%i:
            )
        )
    )

    if "!dataDriver!" == "" (
        call :Log "there is no unused disk driver."
        goto :end
    ) else (
        call :Log "find unused data disk driver !dataDriver!."
    )

    if "%authUser%" == "" (
        net use !dataDriver! \\%storageIP%\%dataPath% /PERSISTENT:YES
    ) else (
        net use !dataDriver! \\%storageIP%\%dataPath% "*" /user:%authUser% /PERSISTENT:YES
        net use \\%storageIP%\%dataPath% /SAVECRED
    )
) else (
    if not "!dataMount!" == "" (
        if not "!dataMount!" == "!dataDriver!" (
            call :Log "dataMount=!dataMount!,dataDriver=!dataDriver!"
            set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
            goto :end
        )
    )
)

if "!logDriver!" == "" (
    rem get unused disk driver
    set str=z y x w v u t s r q p o n m l k j i h g f e d c
    for %%i in (%str%) do (
        if not exist %%i: (
            if not "!logDriver!" == "" (
                logDriver=%%i:
            )
        )
    )

    if "!logDriver!" == "" (
        call :Log "there is no unused disk driver."
        goto :end
    ) else (
        call :Log "find unused log disk driver !logDriver!."
    )

    if "%authUser%" == "" (
        net use !logDriver! \\%storageIP%\%logPath% /PERSISTENT:YES
    ) else (
        net use !logDriver! \\%storageIP%\%logPath% "*" /user:%authUser% /PERSISTENT:YES
        net use \\%storageIP%\%logPath% /SAVECRED
    )
) else (
    if not "!logMount!" == "" (
        if not "!logMount!" == "!logDriver!" (
            call :Log "logMount=!logMount!,logDriver=!logDriver!"
            set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
            goto :end
        )
    )
)

rem write result to result file
echo !dataDriver! >> !RSTFILE!
echo !logDriver! >> !RSTFILE!
call :Log "Finish preparing nas backup medium."
goto :end

rem Print log function, controled by "NEEDLOGFLG".
:Log
    echo %date:~0,10% %time:~0,8% [%username%] [!CURRENTPID!] %~1 >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
goto :EOF

rem Delete file function, it can delete many files.
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
goto :EOF

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )

    set %~2=!CONVERTSTR!
goto :EOF

:end
    call :DeleteFile %DISKPART_CMD_FILE%
    exit !ERR_CODE!

endlocal
