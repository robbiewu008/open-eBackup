@echo off
rem @dest:   application agent for oracle
rem @date:   2020-06-10
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
set EXECSQLRMAN="ExecSQL%PID%.sql"
set EXECSQLRMANRST="ExecSQLRST%PID%.txt"

set PDBSQL="%AGENT_TMP_PATH%PDBSQL%PID%.sql"
set PDBSQLRST="%AGENT_TMP_PATH%PDBSQLRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set LOGFILE=oraclenativemovedbf.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

set DATA_FILES="%AGENT_TMP_PATH%\datafiles%PID%"
set FileSep=\
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
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "DataPath" DATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "MetaDataPath" METADATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "recoverTarget" RECOVERTARGET
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "recoverPath" RECOVERPATH

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)
call :SetDBAuth
call :SetRMANAuth

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativemovedbf.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativemovedbf.bat %AGENT_ROOT% %PID%"
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
set ADDITIONAL=
set fstChar=!DATAPATH:~0,1!
if "!fstChar!" == "+" (
    set "ADDITIONAL=!METADATAPATH!\additional"
    set FileSep=/
) else (
    set "ADDITIONAL=!DATAPATH!\additional"
)

rem check excute binary status
where sqlplus
if !errorlevel! NEQ 0 (
    call :Log "can't find sqlplus"
    set ERR_CODE=1
    goto :error
)
where rman
if !errorlevel! NEQ 0 (
    call :Log "can't find rman"
    set ERR_CODE=1
    goto :error
)

rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;DATAPATH=!DATAPATH!;LOGPATH=!LOGPATH!;METADATAPATH=!METADATAPATH!;RECOVERPATH=!RECOVERPATH!;ADDITIONAL=!ADDITIONAL!"

set ERR_CODE=1
rem check restore parameter
if "!DBINSTANCE!" == "" (
    call :Log "DBINSTANCE is null"
    goto :error
)

copy /y "!ADDITIONAL!\dbfiles" !DATA_FILES!

call :MoveDBFOnline
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

:SetRMANAuth
    if !AUTHMODE!==1 (
        set RMAN_LOGIN=/
    )
    if !AUTHMODE!==0 (
        set RMAN_LOGIN=%DBUSER%/"%DBUSERPWD%"
    )
goto :eof

:CheckDBisCDB
    set MAIN_VERSION=%ORA_VERSION:~0,2%
    if "!MAIN_VERSION!" LSS "12" (
        set %~1=0
        goto :eof
    )

    rem get the huawei backup archive log scn
    echo set linesize 999 > %EXECSQL%
    echo set heading off >> %EXECSQL%
    echo set pagesize 0 >> %EXECSQL%
    echo set feedback off >> %EXECSQL%
    echo select cdb from v$database; >> %EXECSQL%
    echo exit >> %EXECSQL%

    call :Log "Exec sql to get cdb properties of %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get cdb properties of instance failed, don't start cdb."
        set %~1=0
        goto :eof
    )

    set cdbProp=
    for /f "tokens=* delims=" %%a in ('type %EXECSQLRST%') do (
        set cdbProp=%%a
    )
    if "!cdbProp!" == "YES" (
        set %~1=1
    ) else (
        set %~1=0
    )
goto :eof

:MoveDBFOnline
    rem alter database move datafile originfile to newfile;
    call :CheckDBisCDB isCDB

    call :DeleteFile %EXECSQL%
    for /f "tokens=1-4 delims=; " %%a in ('type !DATA_FILES!') do (
        set conId=%%a
        set tsName=%%b
        set fileNo=%%c
        set tsFile=%%d

        set "dataFst=!DATAPATH:~0,1!"
        if "!dataFst!" == "+" (
            call asmcmd ls "!DATAPATH!/FNO-!fileNo!_TS-!tsName!.dbf" >nul
        ) else (
            dir "!DATAPATH!\FNO-!fileNo!_TS-!tsName!.dbf" >nul
        )
        if "!errorlevel!" == "1" (
            call :Log "file FNO-!fileNo!_TS-!tsName!.dbf is not exists."
            set ERR_CODE=1
            goto :error
        )
        
        call "%POWERSHELL_PATH%\powershell.exe" -command "Split-path '!tsFile!'" > %EXECSQLRST%
        set fileDir=
        for /f "tokens=* delims=" %%a in ('type %EXECSQLRST%') do (
            set fileDir=%%a
        )

        if "!isCDB!" == "1" (
            echo set linesize 10 > %PDBSQL%
            echo set heading off >> %PDBSQL%
            echo set pagesize 0 >> %PDBSQL%
            echo set feedback off >> %PDBSQL%
            echo select name from v$pdbs where con_id='!conId!'; >> %PDBSQL%
            echo exit >> %PDBSQL%
            call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %PDBSQL% %PDBSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
            if "!RetCode!" EQU "0" (
                set PDBName=
                for /f "tokens=* delims=" %%a in ('type %PDBSQLRST%') do (
                    set PDBName=%%a
                )
                if not "!PDBName!" == "" (
                    echo alter session set CONTAINER=!PDBName!; >> %EXECSQL%
                )
            ) else (
                call :Log "[WARNING]Query PDB name of !conId! failed."
            )
        )

        if "!RECOVERPATH!" == "" (
            rem echo host echo move datafile FNO-!fileNo!_TS-!tsName!.dbf to !fileDir! ^>^> %LOGFILEPATH%; >> %EXECSQL%
            echo alter database move datafile '!DATAPATH!!FileSep!FNO-!fileNo!_TS-!tsName!.dbf' to '!fileDir!!FileSep!FNO-!fileNo!_TS-!tsName!.dbf'; >> %EXECSQL%
        ) else (
            rem echo host echo move datafile FNO-!fileNo!_TS-!tsName!.dbf to !RECOVERPATH! ^>^> %LOGFILEPATH%; >> %EXECSQL%
            echo alter database move datafile '!DATAPATH!!FileSep!FNO-!fileNo!_TS-!tsName!.dbf' to '!RECOVERPATH!!FileSep!FNO-!fileNo!_TS-!tsName!.dbf'; >> %EXECSQL%
        )
        rem echo host echo move datafile FNO-!fileNo!_TS-!tsName!.dbf successfully ^>^> %LOGFILEPATH%; >> %EXECSQL%
    )
    echo exit; >> %EXECSQL%
    
    type %EXECSQL% >> %LOGFILEPATH%

    call :Log "Exec sql to move data online of %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 864000 RetCode
    if "!RetCode!" EQU "0" (
        call :Log "move data online successfully."
    ) else (
        call :Log "move data online failed, RetCode=!RetCode!."
        set ERR_CODE=!RetCode!
        type %EXECSQLRST% >> %LOGFILEPATH%
        goto :error
    )

goto :eof

:DeleteDBService
    call sc stop OracleService!DBNAME! >> %LOGFILEPATH%
    call %COMMONFUNC% "%AGENT_ROOT%" stopWindowsServices %PID% %LOGFILE% OracleService!DBNAME! RetCode
    if "!RetCode!" == "0" (
        call :Log "stop OracleService!DBNAME! successfully."
    ) else (
        call :Log "stop OracleService!DBNAME! failed, rst=!RetCode!, but continue to delete service."
    )
    call sc delete OracleService!DBNAME! >> %LOGFILEPATH%
    call :Log "delete db service OracleService!DBNAME! successfully."
goto :eof

:StopInstance
    echo shutdown immediate; > %EXECSQL%
    echo exit >> %EXECSQL%
    call :Log "Exec sql to stop instance %DBINSTANCE%."
    call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 600 RetCode
    if "!RetCode!" NEQ "0" (
        if "!RetCode!" NEQ "!ERROR_RECOVER_INSTANCE_NOSTART!" (
            if "!RetCode!" NEQ "!ERROR_ORACLE_NOT_MOUNTED!" (
                if "!RetCode!" NEQ "!ERROR_ORACLE_NOT_OPEN!" (
                    call :Log "Finish stopping instance failed, ret=!RetCode!"
                )
            )
        )
    )
goto :eof

:error
    call :DeleteFile %EXECSQL%
    call :DeleteFile %EXECSQLRST%
    call :DeleteFile %PDBSQL%
    call :DeleteFile %PDBSQLRST%
    call :DeleteFile %DATA_FILES%
    
    if "!ERR_CODE!" NEQ "0" (
       call :StopInstance
       call :DeleteDBService
    )
    exit !ERR_CODE!
endlocal
