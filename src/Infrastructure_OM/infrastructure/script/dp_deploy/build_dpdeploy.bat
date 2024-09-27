@echo off
setlocal DisableDelayedExpansion

cd %WORKSPACE%
mkdir final_package
echo %date%
set zipfile=dpclient_windows.tar
set dirpath=%~dp0
echo %dirpath%

:: check 7zip software
where /q 7z
if errorlevel 1 (
    echo 7-Zip not found
    exit /b 1
)

cd %WORKSPACE%\Infrastructure_OM\infrastructure\script\dp_deploy\client
python build_windows.py %version%

echo "=====================begin_signature===================="
call %WORKSPACE%\Infrastructure_OM\infrastructure\script\dp_deploy\signature_cms.bat dpclient
cd %WORKSPACE%\Infrastructure_OM\infrastructure\script\dp_deploy\client\dpclient
7z a -ttar %zipfile% * > nul
7z a -tgzip  %zipfile%.gz  %zipfile%
copy /Y  *.tar.gz   %WORKSPACE%\final_package
cd %WORKSPACE%
call %WORKSPACE%\Infrastructure_OM\infrastructure\script\dp_deploy\signature_hwp7s.bat *.tar.gz

artget push -d %WORKSPACE%\Infrastructure_OM\infrastructure\conf\dpserver_to_cmc.xml -p "{'componentVersion':'%componentVersion%','CODE_BRANCH':'%branch%'}" -ap %WORKSPACE%\final_package -user %cmc_user% -pwd %cmc_pwd%