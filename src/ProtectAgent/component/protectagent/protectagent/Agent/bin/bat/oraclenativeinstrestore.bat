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
rem @date:   2020-05-06
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
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"

rem check instance status
set EXECSQL="%AGENT_TMP_PATH%ExecSQL%PID%.sql"
set EXECSQLRST="%AGENT_TMP_PATH%ExecSQLRST%PID%.txt"
set EXECSQLRMAN="ExecSQL%PID%.sql"
set EXECSQLRMANRST="ExecSQLRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set LOGFILE=oraclenativeinstrestore.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryServiceRST%PID%.txt"
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
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMSIDNAME
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "DataPath" DATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "LogPath" LOGPATH
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "MetaDataPath" METADATAPATH
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "recoverTarget" RECOVERTARGET
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "recoverPath" RECOVERPATH
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "recoverOrder" RECOVERORDER
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "pitTime" RECOVERTARGET
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "pitScn" PITSCN
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "pitTime" PITTIME
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "dbType" DBTYPE
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "EncAlgo" ENCALGO
call %COMMONFUNC% "%AGENT_ROOT%" getvalue %PID% %LOGFILE% "!INPUTINFO!" "EncKey" ENCKEY

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativeinstrestore.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclenativeinstrestore.bat %AGENT_ROOT% %PID%"
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
    set ADDITIONAL=!METADATAPATH!\additional
    set FileSep=/
) else (
    set ADDITIONAL=!DATAPATH!\additional
)

call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;ASMSIDNAME=!ASMSIDNAME!;ASMUSER=!ASMUSER!;DATAPATH=!DATAPATH!;LOGPATH=!LOGPATH!;METADATAPATH=!METADATAPATH!;RECOVERTARGET=!RECOVERTARGET!;RECOVERPATH=!RECOVERPATH!;RECOVERORDER=!RECOVERORDER!;ADDITIONAL=!ADDITIONAL!;PITSCN=!PITSCN!;PITTIME=!PITTIME!;DBTYPE=!DBTYPE!;ENCALGO=!ENCALGO!"

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

set ERR_CODE=1
rem check restore parameter
if "!DBINSTANCE!" == "" (
    call :Log "DBINSTANCE is null"
    goto :error
)
if "!DATAPATH!" == "" (
    call :Log "DATAPATH is null"
    goto :error
)
if "!LOGPATH!" == "" (
    call :Log "LOGPATH is null"
    goto :error
)
if "!LOGPATH!" == "!DATAPATH!" (
    call :Log "!LOGPATH! is equal to !DATAPATH!"
    goto :error
)
if "!RECOVERTARGET!" == "1" (
    if "!RECOVERPATH!" == "" (
        call :Log "RECOVERTARGET is equal to 1 and RECOVERPATH is empty."
        goto :error
    )
)

rem get restore head pit scn and pit time
set maxScn=
set maxTime=
for /f "tokens=1,2 delims= " %%a in ('type "!ADDITIONAL!\scn_dbf_max"') do (
    set maxScn=%%a
    set maxTime=%%b
)
if "!maxScn!" == "" (
    call :Log "get additional scn_dbf_max maxscn failed."
    goto :error
)
if "!maxTime!" == "" (
    call :Log "get additional scn_dbf_max maxTime failed."
    goto :error
)
call :Log "scn_dbf_max=!maxScn!, maxTime=!maxTime!"

rem check pit paramter
if "!PITSCN!" == "" (
    if "!PITTIME!" == "" (
        set PITSCN=!maxScn!
    )
)
set PIT=
if not "!PITSCN!" == "" (
    if "!PITSCN!" LSS "!maxScn!" (
        call :Log "PITSCN=!PITSCN!, dbf_scn=!maxScn!, pitscn is invalid."
        goto :error
    )
    set PIT=SCN !PITSCN!
) else (
    if "!PITTIME!" LSS "!maxTime!" (
        call :Log "PITTIME=!PITTIME!, maxTime=!maxTime!, pittime is invalid."
        goto :error
    )
    set RESTORE_PTIME=
    call "%POWERSHELL_PATH%\powershell.exe" -command "Get-Date ([timezone]::CurrentTimeZone.ToLocalTime(([datetime]'1/1/1970').AddSeconds(!PITTIME!))) -uformat '%Y-%m-%d %H:%M:%S'" > %EXECSQLRST%
    for /f %%a in ('type %EXECSQLRST%') do (
        set RESTORE_PTIME=%%a
    )
    set PIT=TIME TO_DATE('!RESTORE_PTIME!', 'YYYY-MM-DD HH24:MI:SS')
)

rem try to get ASM instance name
if not "%ASMSIDNAME%" == "" (
    sc query state= all | findstr /i "OracleASMService%ASMSIDNAME%" > %QUREYSERVICE%
    set ASMSIDNAME=+ASM
    for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
        if "SERVICE_NAME"=="%%a" (
            for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
                set ASMSIDNAME=+%%j
            )
        )
    )
    call :DeleteFile %QUREYSERVICE%
    call :Log "check ASM Instance=!ASMSIDNAME!"
)

rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION
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
set DBISCLUSTER=0
rem ************************get grid path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETGRIDPATH% %PID% %LOGFILE% !ORA_VERSION! GRIDHOMEPATH
if "!GRIDHOMEPATH!" == "" (
    call :Log "Get grid home path failed." 
)
rem check if iscluster
if not "!GRIDHOMEPATH!" == "" (
    if exist "!GRIDHOMEPATH!\bin\crsctl.exe" (
        "!GRIDHOMEPATH!\bin\crsctl.exe" check help | findstr /c:"crsctl check crs">nul
        if !errorlevel! EQU 0 (
            set DBISCLUSTER=1
        )
    )
)
call :Log "ORA_VERSION=!ORA_VERSION!;GRIDHOMEPATH=!GRIDHOMEPATH!;ORACLEBASEPATH=!ORACLEBASEPATH!;ORACLEHOMEPATH=!ORACLEHOMEPATH!;DBISCLUSTER=!DBISCLUSTER!"

if "!DBISCLUSTER!" == "1" (
    set ERR_CODE=%ERROR_SCRIPT_EXEC_FAILED%
    call :Log "oracle rac don't supported in windows."
    goto :error
)

rem no needc to test db instance, there is a step
rem set temp file
set DATA_FILES="%AGENT_TMP_PATH%\datafiles%PID%"
set CTL_FILES="%AGENT_TMP_PATH%\ctlfiles%PID%"
set LOG_FILES="%AGENT_TMP_PATH%\logfiles%PID%"
set ORA_PFILE=!ORACLEHOMEPATH!\dbs\ebackup-!DBNAME!-pfile.ora

rem 0:service is not exit; 1:service is exit but not open; 2:service is open
set serviceStatus=0
call :CheckServiceStatus
if "!serviceStatus!" EQU "2" (
    set ERR_CODE=1 
    call :Log "DatabaseService !DBINSTANCE! is open, can't instant restore to host."
    goto :error
)

rem start restore to no exist DB
call :PrepareConfFile

rem replace new environment
call :ReplaceFileByNewEnv

rem create database path if database is not exist
call :PrepareDBEnv

rem modify pfile
call :ModifyPfile

rem create windows service
call :CreateDBService

rem create database spfile for startup
call :CreateSpfile

rem excute database restore
call :ExecDBRestore

call :Log "instant restore instance !DBINSTANCE! successfully."
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

:CheckServiceStatus
    rem check db exists
    set ORACLEDBSERVICE=OracleService!DBINSTANCE!
    sc query %ORACLEDBSERVICE%
    if !errorlevel! NEQ 0 (
        call :Log "The %ORACLEDBSERVICE% Service isn't exists."
        set serviceStatus=0
        goto :eof
    )
	
    rem Check the database exist
    for /f "tokens=2 delims=:" %%i in ('sc query %ORACLEDBSERVICE% ^| findstr /i "STATE"') do (
        set INSTANCESTATUS=%%i
        for /f "tokens=1 delims= " %%j in ("!INSTANCESTATUS!") do (
            set /a oraStatus=%%j
            rem Check Service is Start
            if !oraStatus! EQU 1 (
                call :Log "The %ORACLEDBSERVICE% Service is exists and stopped."
                set serviceStatus=1
                goto :eof
            ) else (
                call :Log "The %ORACLEDBSERVICE% Service Status !oraStatus! is not stopped."
                set serviceStatus=2
                goto :eof
            )
        )
    )
goto :eof

:PrepareConfFile
    rem copy some config file to host
    copy /y "!ADDITIONAL!\ebackup-!DBNAME!-pfile.ora" "!ORA_PFILE!"
    copy /y "!ADDITIONAL!\dbfiles" !DATA_FILES!
    copy /y "!ADDITIONAL!\ctrlfiles" !CTL_FILES!
    copy /y "!ADDITIONAL!\logfiles" !LOG_FILES!
    xcopy /y/s/e "!ADDITIONAL!\dbs\orapw!DBINSTANCE!" "!ORACLEHOMEPATH!\dbs\"
    call :Log "Finish PrepareConfFile"
goto :eof

:ReplaceFileByNewEnv
    rem get origin information
    set originOraBase=
    set originOraHome=
    for /f "tokens=2 delims==" %%a in ('type "!ADDITIONAL!\env_file" ^| findstr "ORACLE_BASE="') do (
        set originOraBase=%%a
    )
    for /f "tokens=2 delims==" %%a in ('type "!ADDITIONAL!\env_file" ^| findstr "ORACLE_HOME="') do (
        set originOraHome=%%a
    )

    if "!originOraBase!" == "" (
        set ERR_CODE=1
        call :Log "originOraBase is null"
        goto :eof
    )

    if "!originOraHome!" == "" (
        set ERR_CODE=1
        call :Log "originOraHome is null"
        goto :eof
    )

    call :ReplaceNewContext !originOraBase! !ORACLEBASEPATH! "!ORA_PFILE!"
    call :ReplaceNewContext !originOraHome! !ORACLEHOMEPATH! "!ORA_PFILE!"
    
    call :ReplaceNewContext !originOraBase! !ORACLEBASEPATH! !DATA_FILES!
    call :ReplaceNewContext !originOraHome! !ORACLEHOMEPATH! !DATA_FILES!

    call :ReplaceNewContext !originOraBase! !ORACLEBASEPATH! !CTL_FILES!
    call :ReplaceNewContext !originOraHome! !ORACLEHOMEPATH! !CTL_FILES!
    
    call :ReplaceNewContext !originOraBase! !ORACLEBASEPATH! !LOG_FILES!
    call :ReplaceNewContext !originOraHome! !ORACLEHOMEPATH! !LOG_FILES!
    call :Log "Finish ReplaceFileByNewEnv"
goto :eof

:ReplaceNewContext
    set tmpfile="%AGENT_TMP_PATH%replacetmp.txt%PID%"
    set oldstr=%~1
    set newstr=%~2
    set replaceFile="%~3"
    call :DeleteFile !tmpfile!
    for /f "tokens=* delims=" %%a in ('type !replaceFile!') do (
        set "str=%%a"
        echo "!str!" | findstr "%oldstr%" >nul
        if !errorlevel! EQU 0 (
            call set "str=!str:%oldstr%=%newstr%!"
        )
        echo !str! >> !tmpfile!
    )

    move !tmpfile! !replaceFile!
goto :eof

:PrepareDBEnv
    set ADUMP_DIR=
    set DIAG_DIR=
    set RECOVERY_DIR=
    for /f "tokens=2,3 delims='" %%a in ('type "!ORA_PFILE!"') do (
        set str=%%a
        echo !str! | findstr "audit_file_dest" >nul
        if !errorlevel! EQU 0 (
            call :CreateDir "%%b"
        )

        echo !str! | findstr "diagnostic_dest" >nul
        if !errorlevel! EQU 0 (
            call :CreateDir "%%b"
        )

        echo !str! | findstr "db_recovery_file_dest" >nul
        if !errorlevel! EQU 0 (
            call :CreateDir "%%b"
        )
    )
    
    call :Log "Finish PrepareDBEnv"
goto :eof

:CreateDir
    rem only support one level directory
    set DirName="%~1"
    set "dirFst=!DirName:~0,1!"
    if "!dirFst!" == "+" (
        set ORACLE_SID=!ASMSIDNAME!
        set ORACLE_HOME=!GRIDHOMEPATH!

        rem create diretory one by one
        set fullDir=
        set tmpDir=
        rem support 10 level directory
        for /l %%a in (1 1 10) do (
            call :getSubDir %%a tmpDir
            if "!tmpDir!" == "" (
                goto :eof
            )
            
            if "!fullDir!" == "" (
                set fullDir=!tmpDir!
            ) else (
                set fullDir=!fullDir!/!tmpDir!
            )
            call asmcmd ls "!fullDir!" >> !LOGFILEPATH!
            if !errorlevel! NEQ 0 (
                call asmcmd mkdir "!fullDir!" >> !LOGFILEPATH!
                if !errorlevel! NEQ 0 (
                    call :Log "create asm folder !DirName! failed."
                    set ERR_CODE=1
                    goto :error
                )
            )
        )
    ) else (
        if not exist "!DirName!" ( md "!DirName!" )
        if !errorlevel! NEQ 0 (
            call :Log "create fs folder !DirName! failed."
            set ERR_CODE=1
            goto :error
        )
    )
goto :eof

:getSubDir
    set %2=
    for /f "tokens=%1 delims=/" %%x in ("%dirName%") do (
        set %2=%%x
    )
goto :eof

:ModifyPfile
    rem use origin position do nothing exclude RECOVERPATH isn't null
    set ctlFile=
    set ctlDir=
    if "!RECOVERPATH!" == "" (
        rem if RECOVERPATH is null
        for /f "tokens=* delims=" %%a in ('type !CTL_FILES!') do (
            rem create control file directory
            call "%POWERSHELL_PATH%\powershell.exe" -command "Split-path '%%a'" > %EXECSQLRST%
            set fileDir=
            for /f "tokens=* delims=" %%a in ('type %EXECSQLRST%') do (
                set fileDir=%%a
            )

            rem set ctlDir with 1st file
            if "!ctlDir!" == "" (
                if not "!fileDir!" == "" (
                    set "ctlDir=!fileDir!"
                    call :CreateDir "!ctlDir!"
                )
            )
        )
    ) else (
        set "ctlDir=!RECOVERPATH!"
    )

    set "ctlFile=!ctlDir!\CONTROLFILE.CTL"
    set tmpfile="%AGENT_TMP_PATH%\replacetmp.txt%PID%"
    call :DeleteFile !tmpfile!
    for /f "tokens=* delims=" %%a in ('type "!ORA_PFILE!"') do (
        set "str=%%a"
        echo "!str!" | findstr "control_files=" >nul
        if !errorlevel! EQU 0 (
            echo *.control_files='!ctlFile!' >> !tmpfile!
        ) else (
            echo !str! >> !tmpfile!
        )
    )
    move !tmpfile! "!ORA_PFILE!"
    call :Log "Finish ModifyPfile."
    
    set dirFst=!ctlDir:~0,1!
    set dataFst=!DATAPATH:~0,1!
    if "!dataFst!" == "+" (
        set ORACLE_SID=!ASMSIDNAME!
        set ORACLE_HOME=!GRIDHOMEPATH!
        call asmcmd cp "!DATAPATH!/CONTROLFILE.CTL" "!ctlDir!"
    ) else (
        if "!dirFst!" == "+" (
            set ORACLE_SID=!ASMSIDNAME!
            set ORACLE_HOME=!GRIDHOMEPATH!
            call asmcmd cp "!DATAPATH!/CONTROLFILE.CTL" "!ctlDir!"
        ) else (
            copy /y "!DATAPATH!\CONTROLFILE.CTL" "!ctlDir!"
        )
    )
    
    call :Log "Finish copying control file to !ctlDir!."
goto :eof

:ExecDBRestore
    call :Log "Begin to exec DB !DBNAME! restore"
    set DBID=
    set DBUName=
    set DBInst=
    set ASMUser=
    set ASMInstance=
    set INCAR_NUM=

    for /f "tokens=1-6 delims=;" %%a in ('type "!ADDITIONAL!\dbinfo"') do (
        set DBID=%%a
        set DBID=!DBID:~1!
        set DBUName=%%b
        set DBUName=!DBUName:~1!
        set DBInst=%%c
        set DBInst=!DBInst:~1!
        set ASMUser=%%d
        set ASMUser=!ASMUser:~1!
        set ASMInstance=%%e
        set ASMInstance=!ASMInstance:~1!
        set INCAR_NUM=%%f
        set INCAR_NUM=!INCAR_NUM:~1!
    )
    if "!DBID!" == "" (
        call :Log "Get Database Id failed."
        type "!ADDITIONAL!\dbinfo" >> %LOGFILEPATH%
        set ERR_CODE=1
        goto :error
    )
    
    rem if asm information is null, incarnation value is asm user
    if "!INCAR_NUM!" == "" (
        set INCAR_NUM=!ASMUser!
    )
    
    if "!INCAR_NUM!" == "" (
        call :Log "Get Database incarnation number failed."
        type "!ADDITIONAL!\dbinfo" >> %LOGFILEPATH%
        set ERR_CODE=1
        goto :error
    )

    set EPOCH_SCN=
    for /f "tokens=1-3 delims= " %%a in ('type "!ADDITIONAL!\scn_epoch"') do (
        set EPOCH_SCN=%%a
    )

    if "!EPOCH_SCN!" == "" (
        call :Log "Get Database epoch scn failed."
        type "!ADDITIONAL!\scn_epoch" >> %LOGFILEPATH%
        set ERR_CODE=1
        goto :error
    )

    echo set dbid=!DBID! > %EXECSQL%
    echo STARTUP MOUNT; >> %EXECSQL%
    echo CATALOG START WITH '!LOGPATH!\EPOCH-SCN_!EPOCH_SCN!\' NOPROMPT; >> %EXECSQL%
    echo CATALOG START WITH '!DATAPATH!\' NOPROMPT; >> %EXECSQL%
    echo RESET DATABASE TO INCARNATION !INCAR_NUM!; >> %EXECSQL%

    rem start to instance restore
    echo RUN >> %EXECSQL%
    echo { >> %EXECSQL%
    
    set dataFileCopy=
    rem construct data file
    for /f "tokens=2-4 delims=;" %%a in ('type !DATA_FILES!') do (
        set tsName=%%a
        set fileNo=%%b
        set tsFile=%%c
        
        echo    SET NEWNAME FOR DATAFILE !fileNo! TO '!DATAPATH!!FileSep!FNO-!fileNo!_TS-!tsName!.dbf'; >> %EXECSQL%
    )

    rem create log file directory
    set /a OMF_ENABLE=0
    set fileDir=
    for /f "tokens=1,2 delims=; " %%a in ('type !LOG_FILES!') do (
        if "!OMF_ENABLE!" EQU "0" (
            set groupNo=%%a
            set logPath=%%b

            call "%POWERSHELL_PATH%\powershell.exe" -command "Split-path '!logPath!' -leaf" > %EXECSQLRST%
            set fileName=
            for /f "tokens=* delims=" %%a in ('type %EXECSQLRST%') do (
                set fileName=%%a
            )

            set "dirFst=!fileName:~0,6!"
            rem O1_MF_ format
            if "!dirFst!" == "O1_MF_" (
                set /a OMF_ENABLE=1
            )

            rem create log file dir
            if not "!RECOVERPATH!" == "" (
                ALTER DATABASE RENAME FILE '!logPath!' TO '!RECOVERPATH!!FileSep!!fileName!'; >> %EXECSQL%
            ) else (
                call "%POWERSHELL_PATH%\powershell.exe" -command "Split-path '!logPath!'" > %EXECSQLRST%
                for /f "tokens=* delims=" %%a in ('type %EXECSQLRST%') do (
                    set "fileDir=%%a"
                )
                call :CreateDir "!fileDir!"
            )
        )
    )

    if "!OMF_ENABLE!" NEQ "0" (
        if not "!RECOVERPATH!" == "" (
            echo    ALTER SYSTEM SET DB_CREATE_FILE_DEST='!RECOVERPATH!'; >> %EXECSQL%
            echo    ALTER SYSTEM SET DB_CREATE_ONLINE_LOG_DEST_1='!RECOVERPATH!'; >> %EXECSQL%
            echo    ALTER SYSTEM SET DB_CREATE_ONLINE_LOG_DEST_2=''; >> %EXECSQL%
        ) else (
            echo    ALTER SYSTEM SET DB_CREATE_FILE_DEST='!fileDir!'; >> %EXECSQL%
            echo    ALTER SYSTEM SET DB_CREATE_ONLINE_LOG_DEST_1='!fileDir!'; >> %EXECSQL%
            echo    ALTER SYSTEM SET DB_CREATE_ONLINE_LOG_DEST_2=''; >> %EXECSQL%
        )
    )

    echo    SET UNTIL !PIT!; >> %EXECSQL%
    echo    RESTORE DATABASE; >> %EXECSQL%
    echo    SWITCH DATAFILE ALL; >> %EXECSQL%
    echo    RECOVER DATABASE;  >> %EXECSQL%
    echo } >> %EXECSQL%
    echo ALTER DATABASE DISABLE BLOCK CHANGE TRACKING; >> %EXECSQL%
    echo ALTER DATABASE OPEN RESETLOGS; >> %EXECSQL%
    echo EXIT >> %EXECSQL%
    
    type %EXECSQL% >> %LOGFILEPATH%
    
    call :Log "Begin to instant restore !DBINSTANCE!, mount first."
    call %COMMONFUNC% "%AGENT_ROOT%" execrmansql %PID% %LOGFILE% %EXECSQLRMAN% %EXECSQLRMANRST% %DBINSTANCE% "!RMAN_LOGIN!" 600 RetCode
    if "!RetCode!" EQU "0" (
        call :Log "Instant restore !DBINSTANCE!, mount successfully."
    ) else (
        call :Log "Instant restore !DBINSTANCE!, mount failed, ret !RetCode!."
        set ERR_CODE=!RetCode!
        goto :error
    )

    call :CheckDBisCDB isCDB
    if "!isCDB!" == "1" (
        echo ALTER PLUGGABLE DATABASE ALL OPEN; > %EXECSQL%
        echo EXIT; >> %EXECSQL%
        
        call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
        if "!RetCode!" NEQ "0" (
            call :Log "[WAR]instant restore:start plugin db !DBINSTANCE! failed, ret !RetCode!."
            type %EXECSQLRMANRST% >> %LOGFILEPATH%
        )
        call :Log "instant restore:start plugin db !DBINSTANCE! successfully."
    )
goto :eof

:CreateSpfile
    call :Log "Begin to create !DBNAME! spfile"
    call :StopInstance

    set SPfile=!ORACLEHOMEPATH!\database\SPFILE!DBINSTANCE!.ora
    set initFile=!ORACLEHOMEPATH!\database\init!DBINSTANCE!.ora
    echo SPFILE='!SPfile!' > %initFile%

    echo STARTUP NOMOUNT PFILE='!ORA_PFILE!'; > %EXECSQL%
    echo CREATE SPFILE='!ORACLEHOMEPATH!\database\SPFILE!DBINSTANCE!.ora' FROM PFILE='!ORA_PFILE!'; >> %EXECSQL%
    echo SHUTDOWN ABORT; >> %EXECSQL%
    echo EXIT; >> %EXECSQL%
    
    call %COMMONFUNC% "%AGENT_ROOT%" execsqls %PID% %LOGFILE% %EXECSQL% %EXECSQLRST% !DBINSTANCE! "!DB_LOGIN!" 90 RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "[ERR]inrestore:create database !DBINSTANCE! spfile failed, ret !RetCode!."
        type %EXECSQLRST% >> %LOGFILEPATH%
        set ERR_CODE=!RetCode!
        goto :error
    )
    call :Log "create !DBNAME! spfile successfully."
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

:SetRMANAuth
    if !AUTHMODE!==1 (
        set RMAN_LOGIN=/
    )
    if !AUTHMODE!==0 (
        set RMAN_LOGIN=%DBUSER%/"%DBUSERPWD%"
    )
goto :eof

:CreateDBService
    if !serviceStatus! EQU 0 (
        call sc create OracleService!DBNAME! binPath= "!ORACLEHOMEPATH!\bin\ORACLE.EXE !DBNAME!" type= own start= demand displayname= "OracleService!DBNAME!" >> %LOGFILEPATH%
    )
    call %COMMONFUNC% "%AGENT_ROOT%" startWindowsServices %PID% %LOGFILE% OracleService!DBNAME! RetCode
    if "!RetCode!" == "0" (
        call :Log "start OracleService!DBNAME! successfully."
    ) else (
        call :Log "start OracleService!DBNAME! failed, rst=!RetCode!."
        set ERR_CODE=!RetCode!
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
    if !serviceStatus! EQU 0 {
        call sc delete OracleService!DBNAME! >> %LOGFILEPATH%
        call :Log "delete db service OracleService!DBNAME! successfully."
    }
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

:WinSleep
    timeout %1 > nul
goto :eof

:error
    if "%ERR_CODE%" NEQ "0" (
        call :StopInstance
       if !serviceStatus! EQU 0 (
           call :DeleteDBService
       )
    )
    call :DeleteFile %EXECSQL%
    call :DeleteFile %EXECSQLRST%
    call :DeleteFile %QUREYSERVICE%
    call :DeleteFile %DATA_FILES%
    call :DeleteFile %CTL_FILES%
    call :DeleteFile %LOG_FILES%
    exit %ERR_CODE%
endlocal
