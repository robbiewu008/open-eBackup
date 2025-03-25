@echo off
rem @dest:   application agent for oracle
rem @date:   2020-05-06
rem @authr:  
rem @modify:

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERR_FILE_IS_EMPTY=8
set /a ERROR_RECOVER_INSTANCE_NOSTART=11
set /a ERROR_ORACLE_NOT_MOUNTED=40
set /a ERROR_ORACLE_NOT_OPEN=41

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"
set CMD_EXECSQL_SILE=execsqls

rem check instance status
set EXECSQL="%AGENT_TMP_PATH%ExecSQL%PID%.sql"
set EXECSQLRST="%AGENT_TMP_PATH%ExecSQLRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set LOGFILE=oraclenativeclivemount.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
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

call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAMETMP
if "!DBNAMETMP!" == "" (
    set DBNAME=
) else (
    call :LowTOUper !DBNAMETMP! DBNAME
)
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
if "!DBUSERL!" == "" (
    set DBUSER=
) else (
    call :UperTOLow !DBUSERL! DBUSER
)
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMSIDNAME
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD

rem ************************get oracle path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETORAPATH% %PID% %LOGFILE% ORACLEBASEPATH ORACLEHOMEPATH
if "!ORACLEBASEPATH!" == "" (
    call :Log "Get Oracle base path failed." 
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :eof
)
if "!ORACLEHOMEPATH!" == "" (
    call :Log "Get Oracle home path failed." 
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    goto :eof
)

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetDBAuth

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativeclivemount.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativeclivemount.bat %AGENT_ROOT% %PID%"
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

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;ASMSIDNAME=!ASMSIDNAME!;ASMUSER=!ASMUSER!"

set ERR_CODE=1
rem check restore parameter
if "!DBINSTANCE!" == "" (
    call :Log "DBINSTANCE is null"
    goto :error
)

rem shutdown database
echo shutdown immediate > %EXECSQL%
echo exit >> %EXECSQL%

call :Log "Exec sql to shutdown instance %DBINSTANCE%."
call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
if "!RetCode!" NEQ "0" (
    if "!RetCode!" NEQ "!ERROR_RECOVER_INSTANCE_NOSTART!" (
        if "!RetCode!" NEQ "!ERROR_ORACLE_NOT_MOUNTED!" (
            if "!RetCode!" NEQ "!ERROR_ORACLE_NOT_OPEN!" (
                call :Log "Finish stopping instance failed, ret=!RetCode!"
                set ERR_CODE=!RetCode!
                goto :error
            )
        )
    )
)
call :Log "stop !DBNAME! successfully."

call sc stop OracleService!DBNAME! >> %LOGFILEPATH%
call sc delete OracleService!DBNAME! >> %LOGFILEPATH%

set SPfile=!ORACLEHOMEPATH!\database\SPFILE!DBINSTANCE!.ora
set initFile=!ORACLEHOMEPATH!\database\init!DBINSTANCE!.ora
call :DeleteFile %SPfile%
call :DeleteFile %initFile%

call :Log "Cancel livemount instance !DBINSTANCE! successfully."
set ERR_CODE=0
goto :error

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

rem Convert str to upper
:LowTOUper
    set CONVERTSTR=%~1
    for %%a in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    set %~2=!CONVERTSTR!
goto :EOF

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

:error
    call :DeleteFile %EXECSQL%
    call :DeleteFile %EXECSQLRST%
    exit %ERR_CODE%
endlocal
