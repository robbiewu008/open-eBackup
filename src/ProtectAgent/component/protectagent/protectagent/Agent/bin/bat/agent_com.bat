@echo off
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERROR_DBUSERPWD_WRONG=10
set /a ERROR_RECOVER_INSTANCE_NOSTART=11

set /a ERROR_INSUFFICIENT_WRONG=15

set /a ERROR_ASM_DBUSERPWD_WRONG=21
set /a ERROR_ASM_INSUFFICIENT_WRONG=22
set /a ERROR_ASM_RECOVER_INSTANCE_NOSTART=23
set /a ERROR_ORACLE_NOARCHIVE_MODE=24
set /a ERROR_ORACLE_OVER_ARCHIVE_USING=25
set /a ERROR_ASM_DISKGROUP_ALREADYMOUNT=26
set /a ERROR_ASM_DISKGROUP_NOTMOUNT=27
set /a ERROR_APPLICATION_OVER_MAX_LINK=28
set /a ERROR_DB_ALREADY_INBACKUP=29
set /a ERROR_DB_INHOT_BACKUP=30
set /a ERROR_DB_ALREADYRUNNING=31
set /a ERROR_DB_ALREADYMOUNT=32
set /a ERROR_DB_ALREADYOPEN=33
set /a ERROR_DB_ARCHIVEERROR=34
set /a ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED=35
set /a ERROR_ORACLE_END_HOT_BACKUP_FAILED=36
set /a ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT=37

set /a ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP=38
set /a ERROR_ASM_NO_STARTUP_TNS=39

set /a ERROR_ORACLE_NOT_MOUNTED=40
set /a ERROR_ORACLE_NOT_OPEN=41
set /a ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED=42
set /a ERROR_ORACLE_TNS_PROTOCOL_ADAPTER=43
set /a ERROR_ORACLE_NOT_INSTALLED=44
set /a ERROR_ORACLE_ANOTHER_STARTING=45
set /a ERROR_ORACLE_DB_BUSY=46

rem no support EnableDelayedExpansion, can't get value of variable if open
rem setlocal EnableDelayedExpansion
set AGENT_ROOT=%~1
set CMDNAME=%~2
set PID=%~3
set LOGFILE=%~4
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set PROCMONITOR_EXIT="%AGENT_TMP_PATH%pmexit!CURRENTPID!.flg"

rem command name list
set CMD_GETVALUE=getvalue
set CMD_STARTSERVICE=startWindowsServices
set CMD_STOPSERVICE=stopWindowsServices

if "!CMDNAME!" == "%CMD_GETVALUE%" (
    FOR /F "tokens=1-2 delims==" %%a in ('type "%~5"') do (
        if "%%a" == "%~6" (
            set %~7=%%b
            goto :EOF
        )
    )
    goto :EOF
)

if "!CMDNAME!" == "%CMD_STARTSERVICE%" (
    set /a COMSTATE=4
    set SERVICENAME=%~5
    
    sc query !SERVICENAME! | findstr /i "STATE"
    if not "!errorlevel!"=="0" (
        call :Log "Service !ServiceName! is not exist."
        set /a %~6 = 1
    )
    
    sc start !SERVICENAME! > nul
    set /a DelayCount=0
    set /a WaitResult=0
    call :WaitService 200 120 !SERVICENAME!
    if "!WaitResult!" == "0" (
        call :Log "Start the sevice !SERVICENAME! service successful."
    ) else (
        call :Log "Start the sevice !SERVICENAME! service failed."
    )
    set /a %~6 = !WaitResult!
    goto :end
)

if "!CMDNAME!" == "%CMD_STOPSERVICE%" (
    set /a COMSTATE=1
    set SERVICENAME=%~5
    
    sc query !SERVICENAME! | findstr /i "STATE"
    if not "!errorlevel!"=="0" (
        call :Log "Service !SERVICENAME! is not exist."
        set /a %~6 = 1
    )
    
    sc stop %ORACLEDBSERVICE% >nul
    set /a DelayCount=0
    set /a WaitResult=0
    call :WaitService 200 120 !SERVICENAME!
    if "!WaitResult!" == "0" (
        call :Log "stop the sevice !SERVICENAME! service successful."
    ) else (
        call :Log "stop the sevice !SERVICENAME! service failed."
    )
    set /a %~6 = !WaitResult!
    goto :end
)

goto :EOF


:WaitService
    set ServiceName=%~3
    rem **********OnceDelayTime units:millisecond**********
    call :Log "Want to Check windows service state is !COMSTATE!"
    if !DelayCount! GEQ %~2 (
        if "!COMSTATE!" EQU "4" (
            call :Log "Start the !ServiceName! service failed."
        ) else (
            call :Log "Stop the !ServiceName! service failed."
        )
        set /a WaitResult=1
        goto :EOF
    )
    
    for /f "tokens=2 delims=: " %%i in ('sc query !ServiceName! ^| findstr /i "STATE"') do (
        set TMP=%%i
        call :Log "Now OracleService state is !TMP!"
        if !TMP! NEQ !COMSTATE! (
            set /a DelayCount+=1
            call :ProcDelay %~1
            goto :WaitService %~1 %~2 !ServiceName!
        )
    )
goto :EOF

:ProcDelay DelayMSec
    for /f "tokens=1-4 delims=:. " %%h in ("%TIME%") do set start=%%h%%i%%j%%k
:ProcwaitLoop
    for /f "tokens=1-4 delims=:. " %%h in ("%TIME%") do set now=%%h%%i%%j%%k
    set /a diff=%now%-%start%
    if %diff% LSS %1 goto ProcwaitLoop
    goto :EOF
:EOF

rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%agent_func.bat" %LOGFILEPATH%
    goto :EOF

rem ************************************************************************
rem function name: DeleteFile()
rem aim:           Delete file function
rem input:         the deleted file
rem output:        
rem ************************************************************************
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
    goto :EOF

:WinSleep
    timeout %1 > nul
    goto :eof

:end
endlocal

