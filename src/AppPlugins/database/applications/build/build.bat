@echo on
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set BASE_PATH=%CurBatPath%..\..\
echo BASE_PATH is %BASE_PATH%

set script_dest_path=%1
set conf_dest_path=%2

xcopy /e/y %BASE_PATH%applications %script_dest_path%

xcopy /e/y %BASE_PATH%applications\conf %conf_dest_path%

rd /s/q %script_dest_path%\build
rd /s/q %script_dest_path%\conf
rd /s/q %script_dest_path%\test
del %script_dest_path%\requirements.txt

endlocal
goto :eof