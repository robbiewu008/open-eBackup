rem @echo offs
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set ScannerRootPath=%CurBatPath%..
echo ScannerRootPath is %ScannerRootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set UPLOAD_BRANCH=%1
)
call :compress_scanner_pkg
artget.exe push -d %CurBatPath%LCRP\conf\pkg_into_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%UPLOAD_BRANCH%','COMPONENT_TYPE':'FS_SCANNER','ARCH':'Windows/'}" -ap %ScannerRootPath%\Scanner.zip -user %cmc_user% -pwd %cmc_pwd%
@REM del /f /q Scanner.zip
goto :eof
endlocal

:compress_scanner_pkg:
   if exist %ScannerRootPath%\Scanner rd /s /Q exist %ScannerRootPath%\Scanner
   MKDIR %ScannerRootPath%\Scanner
   xcopy /y /e  %ScannerRootPath%\libs\*.dll %ScannerRootPath%\Scanner\
   xcopy /y /e  %ScannerRootPath%\libs\*.lib %ScannerRootPath%\Scanner\
   cd %ScannerRootPath%
   7z.exe a Scanner.zip Scanner/
   rd /s /q Scanner
   goto :eof

goto :eof
     echo "Finish"
     Exit