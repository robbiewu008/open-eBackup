rem
rem This file is a part of the open-eBackup project.
rem This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
rem If a copy of the MPL was not distributed with this file, You can obtain one at
rem http://mozilla.org/MPL/2.0/.
rem
rem Copyright (c) [2024] Huawei Technologies Co.,Ltd.
rem
rem THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
rem EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
rem MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
rem
rem @echo off
setlocal enabledelayedexpansion

set zipfile=%1
set zipfile_path=%~n1
set zipfile_name=%~nx1
set full_path=%~d1%~p1%
set PBI_ID=260927445
set HWP7S_ALIAS=CMS_G5_Test_Sign_RSA3072PSS_CN_20220505_HUAWEI
set HWP7S_TIMESTAMPALIAS=CMS_G5_Test_TSA_RSA3072PSS_CN_20220505_HUAWEI
set PRODUCTLINEID=049944
set SIGNATURE_IP=10.29.154.209
set PORT_SECOND=12056
set IPprot=%SIGNATURE_IP%:%PORT_SECOND%
set SIGNATURE_PATH=C:\tool\signature-jenkins-slave-5.1.19-RELEASE

rem check parameters
if "%1"=="" (
    echo Usage:%~nx0 ^<zipfile.zip^>
    exit /b 1
)

call :Crate_Signature_Config
call :Signature
exit /b 0

:Crate_Signature_Config
    del %WORKSPACE%/signconf_hwp7s.xml
    echo ^<signtasks^> > %WORKSPACE%\signconf_hwp7s.xml
    echo 	^<signtask name=^"cms_single^"^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<alias^>%HWP7S_ALIAS%^<^/alias^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<timestampalias^>%HWP7S_TIMESTAMPALIAS%^<^/timestampalias^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<fileset path=^"%full_path%\final_package^"^>^ >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<include^>%zipfile_name%^<^/include^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^</fileset^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<crlfile^>crldata.crl^<^/crlfile^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<hashtype^>1^<^/hashtype^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<padmode^>1^<^/padmode^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<signaturestandard^>5^<^/signaturestandard^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<proxylist^>%IPprot%^<^/proxylist^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<productlineid^>%PRODUCTLINEID%^<^/productlineid^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo 		^<versionid^>%PBI_ID%^<^/versionid^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo   ^<^/signtask^> >> %WORKSPACE%\signconf_hwp7s.xml
    echo ^<^/signtasks^> >> %WORKSPACE%\signconf_hwp7s.xml
    type %WORKSPACE%\signconf_hwp7s.xml
	exit /b

:Signature
    type %WORKSPACE%/signconf_hwp7s.xml
    echo "java -jar %SIGNATURE_PATH%\signature.jar %WORKSPACE%\signconf_hwp7s.xml"
	java -jar %SIGNATURE_PATH%\signature.jar %WORKSPACE%\signconf_hwp7s.xml
    if not %errorlevel% equ 0  (
        echo "ERR signature hwp7s failed."
        exit /b 1
    )
    exit /b
