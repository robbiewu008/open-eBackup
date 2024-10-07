rem @echo offs
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set ModuleRootPath=%CurBatPath%..
echo ModuleRootPath is %ModuleRootPath%
if "%1" EQU "" (
   echo "the branch name is missing"
   exit 1
) else (
   set UPLOAD_BRANCH=%1
)

artget.exe push -d %CurBatPath%LCRP\conf\pkg_into_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%UPLOAD_BRANCH%','COMPONENT_TYPE':'Plugins/Windows','ARCH':'framework_dep_pkg/'}" -ap %ModuleRootPath%\opensrc_platform.zip -user %cmc_user% -pwd %cmc_pwd%
goto :eof
endlocal


goto :eof
     echo "Finish"
     Exit