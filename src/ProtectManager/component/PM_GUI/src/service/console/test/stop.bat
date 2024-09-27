@echo off
setlocal enabledelayedexpansion
set ports=4300
for /f "tokens=1-2" %%a in ('echo %ports%') do (
    call :killProgress "%%a"
)

pause

goto :end

:killProgress
	set port=%~1
	for /f "tokens=1-5" %%a in ('netstat -ano ^| find ":%port%"') do (
		if "%%d" == "LISTENING" (
			set pid=%%e
		)
		
		if "!pid!" == "" (
			echo !pid! not exits
		) else (
			echo pid=!pid!
			taskkill /f /PID !pid!
			echo process !pid! have been killed.
		)
	)
	
	goto :EOF

:end
	endlocal
	exit 0