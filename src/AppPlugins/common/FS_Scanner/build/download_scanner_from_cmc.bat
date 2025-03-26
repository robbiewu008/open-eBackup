rem @echo on
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set RootPath=%CurBatPath%..
echo RootPath is %RootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set DOWNLOAD_BRANCH=%1
)
cd %RootPath%
if exist %RootPath%\libs  rd /S /Q %RootPath%\libs 
if not exist %RootPath%\libs  mkdir %RootPath%\libs 
artget.exe pull -d %CurBatPath%LCRP\conf\module_pkg_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%DOWNLOAD_BRANCH%','COMPONENT_TYPE':'FS_SCANNER','ARCH':'Windows/Scanner.zip'}" -ap %RootPath%\libs -user %cmc_user% -pwd %cmc_pwd%
7z.exe e -y libs/Scanner.zip -olibs/
del /a /f /s libs\Scanner.zip
rd /s /q libs\Scanner

goto :eof
endlocal



goto :eof
     echo "Finish"
     Exit