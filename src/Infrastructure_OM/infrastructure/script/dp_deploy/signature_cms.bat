rem @echo off
setlocal enabledelayedexpansion

set zipfile=%1
set hashfile=sha256sum_sync
:: define Extract dir
set zipfile_path=%~n1
:: define signature parameters
set full_path=%~d1%~p1%~n1
set PBI_ID=260927445
set CMS_ALIAS=CMS_Computing_RSA2048_CN_20220810_Huawei
set PRODUCTLINEID=033402
set SIGNATURE_IP=10.243.192.172
set PORT_SECOND=12055
set IPprot=%SIGNATURE_IP%:%PORT_SECOND%
set SIGNATURE_PATH=C:\tool\signature-jenkins-slave-5.1.19-RELEASE

rem check parameters
if "%1"=="" (
    echo Usage:%~nx0 ^<zipfile.zip^>
    exit /b 1
)

call :Create_SHA_Flie
call :Crate_Signature_Config
call :Signature
exit /b 0
:Crate_Signature_Config
	del %WORKSPACE%/signconf.xml
    echo ^<signtasks^> > %WORKSPACE%\signconf.xml
    echo   ^<signtask name=^"cms_single^"^> >> %WORKSPACE%\signconf.xml
    echo       ^<alias^>%CMS_ALIAS%^<^/alias^> >> %WORKSPACE%\signconf.xml
    echo       ^<file^>%full_path%^\sha256sum_sync^<^/file^> >> %WORKSPACE%\signconf.xml
    echo       ^<crlfile^>%full_path%\crldata.crl^<^/crlfile^> >> %WORKSPACE%\signconf.xml
    echo       ^<hashtype^>1^<^/hashtype^> >> %WORKSPACE%\signconf.xml
    echo       ^<proxylist^>%IPprot%^<^/proxylist^> >> %WORKSPACE%\signconf.xml
    echo       ^<signaturestandard^>5^<^/signaturestandard^> >> %WORKSPACE%\signconf.xml
    echo       ^<productlineid^>%PRODUCTLINEID%^<^/productlineid^> >> %WORKSPACE%\signconf.xml
    echo       ^<versionid^>%PBI_ID%^<^/versionid^> >> %WORKSPACE%\signconf.xml
    echo   ^<^/signtask^> >> %WORKSPACE%\signconf.xml
	echo ^<^/signtasks^> >> %WORKSPACE%\signconf.xml
    type %WORKSPACE%\signconf.xml
	exit /b

:Extract_Package
    echo "Extract_Package %zipfile% ....to %zipfile_path%"
    if exist "%zipfile_path%" rd /s/ q %zipfile_path%
    7z x %zipfile% -o%full_path%
	exit /b

:Create_SHA_Flie
    echo "crate sha256 to %hashfile%"
    del %hashfile% > nul 2>&1
    del crldata.crl > nul 2>&1
    for /r "%full_path%" %%i in (*) do (
        for /f "tokens=1,* delims=" %%a in ('CertUtil -hashfile "%%i" SHA256 ^| findstr /v "SHA256 CertUtil"') do (
            echo %%a %%i >> "%full_path%/%hashfile%"
        )
    )
    ::crate nul file
    type nul > %full_path%/crldata.crl
	exit /b

:Signature
    type %WORKSPACE%/signconf.xml
    echo "java -jar %SIGNATURE_PATH%\signature.jar %WORKSPACE%\signconf.xml"
	java -jar %SIGNATURE_PATH%\signature.jar %WORKSPACE%\signconf.xml
    if not %errorlevel% equ 0  (
        echo "ERR signature cms failed."
        exit /b 1
    )
    exit /b
