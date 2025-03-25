@echo off

rem -init
rem The parameters %1 is passed by Agent. So Please does not change following script.
rem init Macro ,it must need
call %~1\bin\agent_thirdpartyfunc.bat -init  "%~1"  "%~2"

rem --getvalue
rem  if user script have input parameters,you can get parameters value for --getvalue
rem example:
::call %~1\bin\agent_thirdpartyfunc.bat -getvalue "!INPUTINFO!" "InstanceName" DBINSTANCE

rem --log
rem :: you can use parameters -log oupput log to Agent log path 
rem :: example:
call %~1\bin\agent_thirdpartyfunc.bat  -log  "Begin to do something."

rem --when user script execute success exit 0;when user script execute failed ,exit 1

exit 0