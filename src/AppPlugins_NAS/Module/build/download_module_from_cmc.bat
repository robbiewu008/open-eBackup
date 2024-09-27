rem @echo on
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set ModuleRootPath=%CurBatPath%..
echo ModuleRootPath is %ModuleRootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set DOWNLOAD_BRANCH=%1
)
cd %ModuleRootPath%
if exist %ModuleRootPath%\libs  rd /S /Q %ModuleRootPath%\libs 
if not exist %ModuleRootPath%\libs  mkdir %ModuleRootPath%\libs 
artget.exe pull -d %CurBatPath%LCRP\conf\code_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%DOWNLOAD_BRANCH%','COMPONENT_TYPE':'Module','ARCH':'Windows/MODULE_rel.zip'}" -ap %ModuleRootPath%\libs -user %cmc_user% -pwd %cmc_pwd%
7z.exe e -y libs/MODULE_rel.zip -olibs/
del /a /f /s libs\MODULE_rel.zip
rd /s /q libs\MODULE_rel

goto :eof
endlocal



goto :eof
     echo "Finish"
     Exit