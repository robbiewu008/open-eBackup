@echo off 

set AGENT_CMDNAME=%~1
set AGENT_CMD_LOG=-log
set AGENT_CMD_EXIT=-exit
set AGENT_CMD_INIT=-init
set AGENT_CMD_RETINFO=-ret
set AGENT_CMD_GETVAL=-getvalue

if "%AGENT_CMDNAME%" == "%AGENT_CMD_LOG%" (
    call :Log "%~2"
	goto :EOF
 )
if "%AGENT_CMDNAME%" == "%AGENT_CMD_EXIT%" (
     call :Exit %~2 %~3 "%~4" %~5 %~6
	 goto :EOF
    )	
	
if "%AGENT_CMDNAME%" == "%AGENT_CMD_INIT%" (
    echo "==========init===== "
    call :Init "%~2" "%~3"
	goto :EOF
)
if "%AGENT_CMDNAME%" == "%AGENT_CMD_GETVAL%" (
    echo "==========GetValue====="
    call :GetValue "%~2" "%~3" "%~4"
	goto :EOF
)
goto :EOF
rem ************************************************************************
rem function name: Init
rem aim:           Init function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Init
    set AGENT_ROOT=%~1
    set PID=%~2
    set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
    set AGENT_LOG_PATH=%AGENT_ROOT%\log\
    set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
    set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
    set RSTFILE="%AGENT_TMP_PATH%RST%PID%.txt"
    set LOGFILE=thirdparty.log
    set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
    
	set INPUTINFO=
    for /f "delims=" %%a in ('type %PARAM_FILE%') do (
        if not "%%a" == "" (
			set INPUTINFO=%%a
		)
	)
	goto :EOF

rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
	goto :EOF

rem Get the specified value from input argument
:GetValue
    set ARGFILENAME="%AGENT_TMP_PATH%ArgFile%PID%"
    set ArgValue=
    set INPUTINFO=%~1
    set KEYNAME=%~2
    call :start "!INPUTINFO!" "!KEYNAME!"
    set %~3=!ArgValue!
	goto :EOF

:start
	if exist %ARGFILENAME% del %ARGFILENAME%
	set ArgInput=%~1
	set ArgName=%~2
	for /l %%a in (1 1 16) do call :cut %%a

	for /f "tokens=1,2 delims==" %%i in ('type %ARGFILENAME%') do (
		if %%i==!ArgName! (
			set ArgValue=%%j
		)
	)
	if exist %ARGFILENAME% del %ARGFILENAME%
	goto :EOF

rem ************************************************************************
rem function name: Exit
rem aim:           exit function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Exit
    if %~1 == 1 (
        if %~2 == "!AGENT_CMD_LOG!" (
             call :Log "%~3"
        )
        if %~4 == "!AGENT_CMD_RETINFO!" (
            echo "%~5" > %RSTFILE% 
        )
        exit 1
    )
    if %~1 == 0 (
        if %~2 == "!AGENT_CMD_LOG!" (
            call :Log %~3
            exit 0
        )
    )
	goto :EOF