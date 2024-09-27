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
call :compress_module_pkg
artget.exe push -d %CurBatPath%LCRP\conf\pkg_into_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'%UPLOAD_BRANCH%','COMPONENT_TYPE':'Module','ARCH':'Windows/'}" -ap %ModuleRootPath%\MODULE_rel.zip -user %cmc_user% -pwd %cmc_pwd%
del /f /q MODULE_rel.zip
goto :eof
endlocal

:compress_module_pkg:
   if exist %ModuleRootPath%\MODULE_rel rd /s /Q exist %ModuleRootPath%\MODULE_rel
   MKDIR %ModuleRootPath%\MODULE_rel
   xcopy /y /e  %ModuleRootPath%\libs %ModuleRootPath%\MODULE_rel\
   cd %ModuleRootPath%
   7z.exe a MODULE_rel.zip MODULE_rel/
   rd /s /q MODULE_rel
   goto :eof

goto :eof
     echo "Finish"
     Exit