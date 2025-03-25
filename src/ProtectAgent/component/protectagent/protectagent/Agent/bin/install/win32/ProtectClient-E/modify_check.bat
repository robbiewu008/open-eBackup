@echo off
setlocal EnableDelayedExpansion

set CURRENT_PATH=%~dp0
set AGENT_BIN_PATH=%CURRENT_PATH%
set AGENT_ROOT_PATH=%AGENT_BIN_PATH%\..
set AGENT_MANAGER_PATH=%AGENT_ROOT_PATH%\..
set AGENT_PKG_INSTALL_PATH=%AGENT_MANAGER_PATH%\..

set LOG_FILE=%AGENT_ROOT_PATH%\log\modify_pre.log
set LOG_ERR_FILE=%AGENT_ROOT_PATH%\tmp\errormsg.log

set AGENT_MODIFY_PACKAGE_PATH=%AGENT_PKG_INSTALL_PATH%\PushModify\
set AGENT_MODIFY_PACKAGE_PATH_DIVER=%AGENT_MODIFY_PACKAGE_PATH:~0,2%

set WIN_SYSTEM_DISK=%WINDIR:~0,1%
set CURRENT_HOSTSN_FILE=C:\Users\Default\HostSN
if not "%WIN_SYSTEM_DISK%" == "" (
    set CURRENT_HOSTSN_FILE=%WIN_SYSTEM_DISK%:\Users\Default\HostSN
)

set CONF_BACKUP_HOSTSN_FILE=%AGENT_ROOT_PATH%\conf\HostSN

set FREE_SPACE=0
rem Modify package need 2GB(1024 * 1024 * 1024 * 2 = 2147483648)
set FREE_SPACE_MIN=2147483648
set ERR_MODIFY_FAIL_VERIFICATE_PARAMETER=1577209881
set ERR_DISK_FREE_ISLESS_2GB=1677931361
set ERR_MODIFY_FAIL_CHECK_HOSTSN=1577210037
set /a PACKGESIZE=%~1
rem #################### Main process ##########################
call :Log "Start check HostSN."
call :CheckHostSN
if NOT %errorlevel% EQU 0 (
    call :LogError "Failed to check HostSN." %ERR_MODIFY_FAIL_CHECK_HOSTSN%
    exit %ERR_MODIFY_FAIL_CHECK_HOSTSN%
)

call :Log "Start create download directory."
call :CheckDownloadDir
if NOT %errorlevel% EQU 0 (
    call :LogError "Failed to create download directory." %ERR_MODIFY_FAIL_VERIFICATE_PARAMETER%
    exit %ERR_MODIFY_FAIL_VERIFICATE_PARAMETER%
)
call :Log "Start check disk space.packgeSize=%PACKGESIZE%"
call :CheckFreeDiskSpace
if NOT %errorlevel% EQU 0 (
    call :LogError "Failed to check free disk space." %ERR_MODIFY_FAIL_VERIFICATE_PARAMETER%
    exit %ERR_DISK_FREE_ISLESS_2GB%
)

endlocal
exit 0

rem #################### Function ##########################
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOG_FILE%"
goto :EOF

:LogError
    call :Log "[ERR] %~1"
    (echo logDetail=%~2) > %LOG_ERR_FILE%
    (echo logInfo=job_log_agent_storage_modify_prepare_fail_label) >> %LOG_ERR_FILE%
    (echo logDetailParam=%~3) >> %LOG_ERR_FILE%
goto :EOF

:CheckDownloadDir
    if exist "%AGENT_MODIFY_PACKAGE_PATH%" (
        call :Log "%AGENT_MODIFY_PACKAGE_PATH% is exist, prepare to clean up the directory."
        rd /s /q  "%AGENT_MODIFY_PACKAGE_PATH%" > nul
    )
    md "%AGENT_MODIFY_PACKAGE_PATH%"
    if NOT exist "%AGENT_MODIFY_PACKAGE_PATH%" (
        call :Log "%AGENT_MODIFY_PACKAGE_PATH% is not exist, create the modify package download directory failed."
        exit /b 1
    )
    exit /b 0
goto :EOF

:CheckFreeDiskSpace
    call :Log "Check free space of DataBackup Agent installation."
    call :Log "Modify package needs free space is 2GB."
    for /f "delims== tokens=1,2" %%a in ('wmic LogicalDisk where "Caption='%AGENT_MODIFY_PACKAGE_PATH_DIVER%'" get FreeSpace /value') do (
        if "%%a" EQU "FreeSpace" (
            set FREE_SPACE=%%b
        )
    )
    if "%FREE_SPACE%" EQU "" (
        call :Log "Free space is empty, please check the set installation package download path."
        exit /b 1
    )
    set /a PACKGESIZE=%PACKGESIZE%*4
    if  "%FREE_SPACE_MIN%" LSS "%PACKGESIZE%" (
        set /a FREE_SPACE_MIN=%PACKGESIZE%
    )
    call :Log "Current diver[%AGENT_MODIFY_PACKAGE_PATH_DIVER%] have [%FREE_SPACE%]Byte free spaceis "
    call :CompareDiskSpace "%FREE_SPACE%" "%FREE_SPACE_MIN%"
    if %errorlevel% NEQ 0 (
        call :Log "Check free space on specified diver failed, the free space is [%FREE_SPACE%]Byte and minimum requirement is [%FREE_SPACE_MIN%]."
        exit /b 1
    )
    call :Log "Check free disk space successfully."
    exit /b 0
goto :EOF

:StrLength
    set count=0
    set inStr=%~1
    :loop
    if not "!inStr:~%count%,1!" == "" (
        set /a count+=1
        goto :loop
    )
    exit /b %count%

rem usage : CompareNumber 123456 123456 
rem input ：tow number (type：string; Require num1,num2 > 0)
rem output：1(num1 > num2) 0(num1 = num2) -1(num1 < num2)
rem Use this function when num1 or num2 overflow (0 ~ 2^31 - 1)
:CompareDiskSpace
    set curDiskSpace=%~1
    set minDiskSpace=%~2
    if not defined curDiskSpace (
        call :Log "curDiskSpace empty"
        exit /b 1
    )

    if not defined minDiskSpace (
        call :Log "minDiskSpace is empty"
        exit /b 1
    )
    call :StrLength %curDiskSpace%
    set curLen=%errorlevel%
    call :StrLength %minDiskSpace%
    set minLen=%errorlevel%
    call :Log "curDiskSpace=%curDiskSpace%:%curLen%, minDiskSpace=%minDiskSpace%:%minLen%"
 
    if %curLen% LSS %minLen% (
        call :Log "length less."
        exit /b 1
    ) else if %curLen% EQU %minLen% (
        call :Log "length eq."
        if "%curDiskSpace%" LEQ "%minDiskSpace%" (
            call :Log "data less."
            exit /b 1
        )
    )
    exit /b 0

:CheckHostSN
    if NOT exist "%CURRENT_HOSTSN_FILE%" (
        call :Log "%CURRENT_HOSTSN_FILE% is missing."
        exit /b 1
    )
    if NOT exist "%CONF_BACKUP_HOSTSN_FILE%" (
        call :Log "%CONF_BACKUP_HOSTSN_FILE% is missing."
        exit /b 1
    )
    for /f "tokens=* delims=" %%a in ('type "%CURRENT_HOSTSN_FILE%"') do (
        if "%%a" EQU "" (
            call :Log "%CURRENT_HOSTSN_FILE% value is NULL."
            exit /b 1
        )
        set current_hostsn=%%a
    )
    for /f "tokens=* delims=" %%a in ('type "%CONF_BACKUP_HOSTSN_FILE%"') do (
        if "%%a" EQU "" (
            call :Log "%CONF_BACKUP_HOSTSN_FILE% value is NULL."
            exit /b 1
        )
        set backup_hostsn=%%a
    )
    if NOT "%current_hostsn%" EQU "%backup_hostsn%" (
        call :Log "backup_hostsn(%backup_hostsn%) is not equal current_hostsn(%current_hostsn%)."
        exit /b 1
    )
    call :Log "backup_hostsn(%backup_hostsn%) is equal current_hostsn(%current_hostsn%)."
    exit /b 0