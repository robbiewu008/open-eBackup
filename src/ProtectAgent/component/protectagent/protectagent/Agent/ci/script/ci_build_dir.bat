@echo off
setlocal EnableDelayedExpansion

set CurBatPath=%~dp0

set BUILD_PKG_TYPE=%~1
set BUILD_OS_TYPE=%~2
PKG_TYPE=Plugins

set BASE_PATH=%CurBatPath%../../../
set OPENSOURCE_REPOSITORY_DIR=%CurBatPath%../../../../../../../open-source-obligation

if not exist "%BASE_PATH%temp" (
    md  "%BASE_PATH%temp"
)

if not exist "%BASE_PATH%Plugins" (
    md  "%BASE_PATH%Plugins"
)

if not exist "%BASE_PATH%tmp_zip" (
    md  "%BASE_PATH%tmp_zip"
)

if not exist "%BASE_PATH%final_pkg" (
    md  "%BASE_PATH%final_pkg"
)

if not exist "%BASE_PATH%final_pkg\ProtectClient-e" (
    md  "%BASE_PATH%"
)

if not exist "%BASE_PATH%final_pkg\PackageScript" (
    md  "%BASE_PATH%final_pkg\PackageScript"
)

if not exist "%BASE_PATH%final_pkg\Plugins" (
    md  "%BASE_PATH%final_pkg\Plugins"
)

if not exist "%BASE_PATH%final_pkg\package" (
    md  "%BASE_PATH%final_pkg\package"
)

if not exist "%BASE_PATH%final_pkg\PackageScript\windows" (
    md  "%BASE_PATH%final_pkg\PackageScript\windows"
)

if not exist "%BASE_PATH%final_pkg\PackageScript\like-unix" (
    md  "%BASE_PATH%final_pkg\PackageScript\like-unix"
)

if not exist "%BASE_PATH%final_pkg\third_party_software" (
    md  "%BASE_PATH%final_pkg\third_party_software"
)


rem #################### 拷贝第三方软件 ##########################

xcopy /y /e "%BASE_PATH%Agent\ci\script\PackingRules" "%BASE_PATH%final_pkg\package"

xcopy /y /e "%OPENSOURCE_REPOSITORY_DIR%\Plugins\Windows\FilePlugin.zip" "%BASE_PATH%Plugins"
xcopy /y /e "%OPENSOURCE_REPOSITORY_DIR%\Plugins\Windows\VirtualizationPlugin.zip" "%BASE_PATH%Plugins"
xcopy /y /e "%OPENSOURCE_REPOSITORY_DIR%\Plugins\Windows\GeneralDBPlugin.zip" "%BASE_PATH%Plugins"
xcopy /y /e "%OPENSOURCE_REPOSITORY_DIR%\Plugins\Windows\ADDSPlugin.zip" "%BASE_PATH%Plugins"

for %%f in ("%BASE_PATH%Agent\bin\install\*.txt" "%BASE_PATH%Agent\bin\install\*.bat") do xcopy /y /e %%f  "%BASE_PATH%tmp_zip"

"%BASE_PATH%Agent\third_party_groupware\7Zip\7z.exe" a "%BASE_PATH%final_pkg\PackageScript\package-windows.zip" "%BASE_PATH%tmp_zip\*"

del /f /q %BASE_PATH%tmp_zip\*
xcopy /y /e "%BASE_PATH%Agent\bin\bat\push_install_check.bat" "%BASE_PATH%final_pkg\PackageScript\windows"

xcopy /y /e "%OPENSOURCE_REPOSITORY_DIR%\ThirdParty\Windows2012\x64\third_party_groupware\OceanProtect_X8000_1.2.1RC1_Opensrc_3rd_SDK_Windows2012_x64.zip" "%BASE_PATH%Agent\open_src"

"%BASE_PATH%Agent\third_party_groupware\7Zip\7z.exe" x "%OPENSOURCE_REPOSITORY_DIR%\ThirdParty\Windows2012\x64\third_party_groupware\OceanProtect_X8000_1.2.1RC1_Opensrc_3rd_SDK_Windows2012_x64.zip" "7zip/lib/7z.*" -o"%BASE_PATH%/tmp_zip/unzip"

if not exist "%BASE_PATH%tmp_zip/7Zip" (
    md  "%BASE_PATH%tmp_zip/7Zip"
)
xcopy /y /e "%BASE_PATH%tmp_zip/unzip/7zip/lib/7z.*" "%BASE_PATH%tmp_zip/7Zip"
"%BASE_PATH%Agent\third_party_groupware\7Zip\7z.exe" a "%BASE_PATH%final_pkg\third_party_software\3rd-windows.zip" "%BASE_PATH%tmp_zip\*"
del /f /q %BASE_PATH%tmp_zip\*