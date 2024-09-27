@echo off
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set BUILD_LOG="%CurBatPath%build.log"
set BASE_PATH=%CurBatPath%..\
echo BASE_PATH is %BASE_PATH% >> %BUILD_LOG%

set SOLUTION=%BASE_PATH%vsprj\databasePlugin.sln
set DB_INC_PATH=%BASE_PATH%inc
set DB_LIB_PATH=%BASE_PATH%lib
set DB_SRC_PATH=%BASE_PATH%src
set DB_BUILD_PATH=%BASE_PATH%vsprj\Release

set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

set DEVEVN_FILE=%VCTOOL_HOME%\Common7\IDE\devenv.exe
echo DEVEVN_FILE is %DEVEVN_FILE% >> %BUILD_LOG%

set VCVARSX86_AMD64_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvarsx86_amd64.bat
echo VCVARSX86_AMD64_BAT is %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%

call :main
endlocal
goto :eof

rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
:FindVCTOOLHome
    set VCTOOLTemp=n/a

    IF EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        set	"VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
        echo Find vswhere.exe suceess. >> %BUILD_LOG%
    ) ELSE (
        echo Not find vswhere.exe. >> %BUILD_LOG%
    )

    rem Query the VS2017 installationPath with vswhere.exe
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set VCTOOLTemp=%%i
    )

    if "%VCTOOLTemp%" NEQ "n/a" (set %1="%VCTOOLTemp%")
goto :eof

:FileNotFound
echo  %DATE% %TIME% %1 Not Found >> %BUILD_LOG%
type %BUILD_LOG%
exit 1

:MakePlugin
    %Drive%
    rem setting the enviroment params with VC command amd64
    call %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%
    
    cd %CurBatPath%
    set ACTION=Rebuild
    set SOLUTION_CONFIG="Release|x64"

    echo Begin to clean compile >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SOLUTION%" /clean >> %BUILD_LOG%
    %DEVEVN_FILE% "%SOLUTION%" /clean

    echo Begin to rebuild the databasePlugin >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv >> %BUILD_LOG%
    %DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

    if %errorlevel% NEQ 0 (
        echo build x64 databasePlugin error! >> %BUILD_LOG%
        type %BUILD_LOG%
        exit 1
    )
    echo Build x64 databasePlugin success. >> %BUILD_LOG%
goto :eof

:CopyFilesToPath
    if not exist %DB_INC_PATH% mkdir %DB_INC_PATH%
    if not exist %DB_LIB_PATH% mkdir %DB_LIB_PATH%

    ::why just job dir ?
    xcopy %BASE_PATH%\src\job\*.h %DB_INC_PATH% /e/y
    if %errorlevel% NEQ 0 (
        echo xcopy %BASE_PATH%\src\job\*.h %DB_INC_PATH% error! >> %BUILD_LOG%
        type %BUILD_LOG%
        exit 1
    )

    xcopy %DB_BUILD_PATH%\*.dll %DB_LIB_PATH% /e/y
    if %errorlevel% NEQ 0 (
        echo xcopy %DB_BUILD_PATH%\*.dll %DB_LIB_PATH% error! >> %BUILD_LOG%
        type %BUILD_LOG%
        exit 1
    )
goto :eof

:main
    if "%1" EQU "clean" (
        echo DB_BUILD_PATH is %DB_BUILD_PATH%. >> %BUILD_LOG%
        rmdir /s/q %DB_BUILD_PATH%
        if %errorlevel% NEQ 0 (
            echo rmdir /s/q %DB_BUILD_PATH% error! >> %BUILD_LOG%
            type %BUILD_LOG%
            exit 1
        )
        echo clean success. >> %BUILD_LOG%
        type %BUILD_LOG%
        exit 0
    )

    cd /d "%DB_SRC_PATH%"
    git-bash.exe -c "find -name *.cpp | xargs unix2dos;find -name *.h | xargs unix2dos;"
    if %errorlevel% NEQ 0 (
        echo Trans LF to CRLF error! >> %BUILD_LOG%
        type %BUILD_LOG%
        exit 1
    )

    echo #######################################################################################################
    echo Start compile DbPlugin.
    echo Start compile DbPlugin. >> %BUILD_LOG%
    echo #######################################################################################################
    call :MakePlugin
    call :CopyFilesToPath
    echo #######################################################################################################
    echo DbPlugin compile success.
    echo DbPlugin compile success. >> %BUILD_LOG%
    echo #######################################################################################################
    
    cd /d "%DB_SRC_PATH%"
    git-bash.exe -c "find -name *.cpp | xargs dos2unix;find -name *.h | xargs dos2unix;"
    if %errorlevel% NEQ 0 (
        echo Trans CRLF to LF error! >> %BUILD_LOG%
        type %BUILD_LOG%
        exit 1
    )

    type %BUILD_LOG%
goto :eof