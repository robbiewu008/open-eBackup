@echo off
setlocal EnableDelayedExpansion

set CurBatPath=%~dp0
set BASE_PATH=%CurBatPath%../../../
set BUILD_PKG_TYPE=%~1
set BUILD_OS_TYPE=%~2

set SIGN_TOOL_PATH=%SIGNATURE_HOME%
echo SIGN_TOOL_PATH=%SIGN_TOOL_PATH%

set OPENSOURCE_REPOSITORY_DIR=%CurBatPath%../../../../../../../open-source-obligation

for %%f in ("%BASE_PATH%temp\protectclient*Windows*.zip") do xcopy /y /e %%f  "%BASE_PATH%final_pkg\ProtectClient-e"

xcopy /y /e "%BASE_PATH%Agent/ci/script/package.json" "%BASE_PATH%"

set "search=<MS_IMAGE_TAG>"
set "replace=%MS_IMAGE_TAG%"
set "file=${BASE_PATH}/package.json"
set "tempfile=%file%.tmp"
(
  for /f "delims=" %%i in ('type "%file%"') do (
    set "line=%%i"
    set "line=!line:%search%=%replace%!"
    echo !line!
  )
) > "%tempfile%"
move /y "%tempfile%" "%file%" >nul


for /f %%x in ('wmic path win32_utctime get /format:list ^<nul ^| findstr "="') do (
    set %%x)
set /a z=(14-100%Month%%%100)/12, y=10000%Year%%%10000-z
set /a ut=y*365+y/4-y/100+y/400+(153*(100%Month%%%100+12*z-3)+2)/5+Day-719469
set /a ut=ut*86400+100%Hour%%%100*3600+100%Minute%%%100*60+100%Second%%%100
del /s/q TempWmicBatchFile.bat >nul 2>&1

set "search=<TIMESTAMP_STARTTIME>"
set "replace=%ut%"
set "tempfile=%file%.tmp"
(
  for /f "delims=" %%i in ('type "%file%"') do (
    set "line=%%i"
    set "line=!line:%search%=%replace%!"
    echo !line!
  )
) > "%tempfile%"
move /y "%tempfile%" "%file%" >nul

set "search=<VERSION>"
set "replace=%version%"
set "tempfile=%file%.tmp"
(
  for /f "delims=" %%i in ('type "%file%"') do (
    set "line=%%i"
    set "line=!line:%search%=%replace%!"
    echo !line!
  )
) > "%tempfile%"
move /y "%tempfile%" "%file%" >nul


"%BASE_PATH%Agent\third_party_groupware\7Zip\7z.exe" a "%OPENSOURCE_REPOSITORY_DIR%\%BUILD_PKG_TYPE%\linux\DataProtect_%Version%_client.zip" "%BASE_PATH%final_pkg\*"