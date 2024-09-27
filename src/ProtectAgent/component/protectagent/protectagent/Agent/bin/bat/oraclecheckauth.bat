@echo off
rem @dest:   application agent for oracle
rem @date:   2020-04-20
rem @authr:  
rem @modify:

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERROR_RECOVER_INSTANCE_NOSTART=11
set /a ERR_FILE_IS_EMPTY=8

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"
set CMD_GETVALUE=getvalue
set CMD_EXECSQL_SILE=execsqls
set CMD_EXECASMSQL_SIL=execasmsqls

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclecheckauth.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryService%PID%.txt"
rem check username password
set CHECKUSERPWD="%AGENT_TMP_PATH%CheckUserPwd%PID%.sql"
set CHECKUSERPWDRST="%AGENT_TMP_PATH%CheckUserPwdRST%PID%.txt"
rem #############################################################

set INPUTINFO=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTINFO=%%a
    )
)
call :DeleteFile %PARAM_FILE%

if "!INPUTINFO!" == "" (
    call :Log "INPUTINFO is null."
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :error
)

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMINSTANCENAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetDBAuth
call :SetASMAuth

rem get current process id for excute sql timeout and log
call wmic process where name="cmd.exe" get processid > %CURRENTPIDRST%
set /a NUM=0
for /f %%a in ('type %CURRENTPIDRST%') do (
    if !NUM! NEQ 0 (
        set processID=%%a
        call wmic process where processid="!processID!" get Commandline > %CURRENTCMDLineRST%
        set AGENT_ROOT_TMP=%AGENT_ROOT: =%
        set BLACKFLAG=1
        if "!AGENT_ROOT_TMP!" == "!AGENT_ROOT!" (
            set BLACKFLAG=0
        )
        if "!BLACKFLAG!" == "1" (
            more %CURRENTCMDLineRST% | findstr /c:"oraclecheckauth.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclecheckauth.bat %AGENT_ROOT% %PID%"
        )
        if !errorlevel! EQU 0 (
            set /a CURRENTPID=%%a
        )
    )
    set /a NUM=!NUM!+1
)

call :DeleteFile %CURRENTPIDRST%
call :DeleteFile %CURRENTCMDLineRST%
if !CURRENTPID! EQU 0 (
    call :Log "CURRENTPID equal 0, the timeout is will failed."
)
set monitorPIDs="%AGENT_TMP_PATH%ProcMonitorLists!CURRENTPID!.txt"

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;ASMINSTANCENAME=!ASMINSTANCENAME!;ASMUSER=!ASMUSER!"
call :DeleteFile %RSTFILE%

call :Log "Begin to check oracle auth"
call :TestDBConnect
if not "!ASMINSTANCENAME!" == "" (
    call :TestASMInstanceConnect
)

rem TODO check privilege
call :Log "Finish checking database authentication"
exit 0

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

:TestDBConnect
    rem check asm intance status
    call :Log "Start to check oracle instance status."
    call :GetInstanceStatus INSTStatus RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get instance status failed."
        exit !RetCode!
    )

    rem STARTED - After STARTUP NOMOUNT
    rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    rem OPEN - After STARTUP or ALTER DATABASE OPEN
    rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if not "!INSTStatus:~0,4!" == "OPEN" (
        call :Log "Instance status !INSTStatus! no open"
        exit %ERROR_RECOVER_INSTANCE_NOSTART%
    )
    call :Log "Test %DBNAME% successful."
    goto :eof

:TestASMInstanceConnect
    rem try to get ASM instance name
    sc query state= all | findstr /i "OracleASMService!ASMINSTANCENAME!" > %QUREYSERVICE%
    for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
        if "SERVICE_NAME"=="%%a" (
            for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
                call :Log "get asm instance !ASMINSTANCENAME!"
                set ASMINSTANCENAME=+%%j
            )
        )
    )
    call :DeleteFile %QUREYSERVICE%
    call :Log "check ASM Instance=!ASMINSTANCENAME!"

    echo select status from v$instance; > %CHECKUSERPWD%
    echo exit >> %CHECKUSERPWD%

    call :Log "Exec sql to get status of asm instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL_SIL% %PID% %LOGFILE% %CHECKUSERPWD% %CHECKUSERPWDRST% %ASMINSTANCENAME% "!ASM_LOGIN!" 30 RetCode
    call :DeleteFile %CHECKUSERPWD%
    call :DeleteFile %CHECKUSERPWDRST%
    if "!RetCode!" NEQ "0" (
        call :Log "Test ASM instance failed."
        exit !RetCode!
    ) else (
        call :Log "Test ASM instance successful."
    )
goto :eof

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo set linesize 600 > %CHECKINSTANCESTATUS%
    echo set pagesize 0 >> %CHECKINSTANCESTATUS%
    echo set feedback off >> %CHECKINSTANCESTATUS%
    echo select status from v$instance; >> %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

    call :Log "Exec sql to get status of instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL_SILE% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        set /a %~2 = !RetCode!
        call :DeleteFile %CHECKINSTANCESTATUS%
        call :DeleteFile %CHECKINSTANCESTATUSRST%
    ) else (
        rem STARTED - After STARTUP NOMOUNT
        rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        rem OPEN - After STARTUP or ALTER DATABASE OPEN
        rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        for /f "delims=" %%a in ('type %CHECKINSTANCESTATUSRST%') do (
            if not "%%a" == "" (
                set %~1=%%a
                set /a %~2 = 0
            )
        )
        call :DeleteFile %CHECKINSTANCESTATUS%
        call :DeleteFile %CHECKINSTANCESTATUSRST%
    )

goto :eof

:SetDBAuth
    if !AUTHMODE!==1 (
        set DB_LOGIN=/ as sysdba
    )
    if !AUTHMODE!==0 (
        if "%DBUSER%"=="sys" (
            set DB_LOGIN=%DBUSER%/"%DBUSERPWD%" as sysdba
        ) else (
            set DB_LOGIN=%DBUSER%/"%DBUSERPWD%"
        )
    )
goto :eof

:SetASMAuth
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=%DBUSER%/"%DBUSERPWD%" as sysasm
    )
goto :eof

:error
    exit /b %ERR_CODE%
    
:end
endlocal
