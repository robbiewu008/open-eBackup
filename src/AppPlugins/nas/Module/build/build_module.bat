rem @echo on
setlocal EnableDelayedExpansion

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set MODULE_PATH=%CurBatPath%..\..\
echo MODULE_PATH is %MODULE_PATH%

rem create windows compile folder
cd "%CurBatPath%"
if exist %MODULE_PATH%Windows_Compile  rd /S /Q "%MODULE_PATH%Windows_Compile"
IF not EXIST %MODULE_PATH%Windows_Compile   MKDIR  %MODULE_PATH%Windows_Compile
IF not EXIST %MODULE_PATH%Windows_Compile\vsprj   MKDIR  %MODULE_PATH%Windows_Compile\vsprj
IF not EXIST %MODULE_PATH%Windows_Compile\Module   MKDIR  %MODULE_PATH%Windows_Compile\Module
IF not EXIST %MODULE_PATH%Windows_Compile\vsprj\bin   MKDIR  %MODULE_PATH%Windows_Compile\vsprj\bin

@REM rem Copy external pkg
cd "%CurBatPath%"

xcopy /y /e %MODULE_PATH%\Module\vsprj  %MODULE_PATH%\Windows_Compile\vsprj
xcopy /y /e %MODULE_PATH%\Module  %MODULE_PATH%\Windows_Compile\Module > nul

call %MODULE_PATH%Windows_Compile\Module\build\download_3rd.bat

xcopy /y /e %MODULE_PATH%Windows_Compile\Module\open_src\all_libs  %MODULE_PATH%\Windows_Compile\vsprj\bin\bin\


cd "%MODULE_PATH%Windows_Compile%"
git-bash.exe -c "cd Module/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"

set SOLUTION=%MODULE_PATH%Windows_Compile\vsprj\compile\build\Module.sln
IF not EXIST %SOLUTION%  call :FileNotFound

echo SOLUTION is %SOLUTION%

set BUILD_LOG="%MODULE_PATH%Windows_Compile\build.log"
echo BUILD_LOG is %BUILD_LOG%

set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

set VCVARS32_BAT="%VCTOOL_HOME%\VC\Auxiliary\Build\vcvars32.bat"
echo VCVARS32_BAT is %VCVARS32_BAT% >> %BUILD_LOG%

set VCVARSX86_AMD64_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvarsx86_amd64.bat
echo VCVARSX86_AMD64_BAT is %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%

set DEVEVN_FILE=%VCTOOL_HOME%\Common7\IDE\devenv.exe
echo DEVEVN_FILE is %DEVEVN_FILE% >> %BUILD_LOG%

echo include is %INCLUDE%

echo 1 is "%1"

call :Compile64PluginFramework


rem Copy lib into Module/libs
IF EXIST %MODULE_PATH%\Module\libs  rd /S /Q "%MODULE_PATH%\Module\libs"
IF not EXIST %MODULE_PATH%\Module\libs MKDIR  %MODULE_PATH%\Module\libs
COPY /Y %MODULE_PATH%Windows_Compile\vsprj\bin\bin\*.dll      %MODULE_PATH%\Module\libs
COPY /Y %MODULE_PATH%Windows_Compile\vsprj\bin\bin\*.lib      %MODULE_PATH%\Module\libs
COPY /Y %MODULE_PATH%Windows_Compile\vsprj\bin\bin\*.pdb      %MODULE_PATH%\Module\libs
COPY /Y %MODULE_PATH%Windows_Compile\vsprj\bin\bin\*.exe      %MODULE_PATH%\Module\libs

echo End of Compile


goto :eof



:Compile64PluginFramework
    %Drive%
    rem build src
    cd %CurBatPath%
    set SOLUTION_CONFIG="Release|x64"
    set ACTION=Rebuild
	call %VCVARSX86_AMD64_BAT%
    echo Begin to clean compile >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SOLUTION%" /clean

    %DEVEVN_FILE% %SOLUTION% /clean
    echo Begin to rebuild the Plugin >> %BUILD_LOG%
    echo %DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv

    %DEVEVN_FILE% %SOLUTION% /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%
    set res=%errorlevel%
    type %MODULE_PATH%Windows_Compile\build.log
    if %res% NEQ 0 (
        echo compile error, pls check
        exit 1    
    )
    echo End to build x64 Plugin
goto :eof


rem
rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
rem
:FindVCTOOLHome
    set VCTOOLTemp=n/a
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        set	VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
        echo Find vswhere.exe suceess. >> %BUILD_LOG%
        echo Find vswhere.exe success.
    ) else (
        echo Not find vswhere.exe. >> %BUILD_LOG%
        echo Not find vswhere.exe.
    )
        
    rem Query the VS2017 installationPath with vswhere.exe
    for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set VCTOOLTemp=%%i
    )

    if "%VCTOOLTemp%" NEQ "n/a" (set %1="%VCTOOLTemp%")
goto :eof

rem
rem Delete a file
rem
:DeleteFile
    set FileName=%~1
    if exist "%FileName%" (del /f /q "%FileName%")
goto :eof

:FileNotFound
    echo  %DATE% %TIME% %1 Not Found,Compile Failed. >> %BUILD_LOG%
    exit 1

goto :eof
     echo "Finish"
     Exit
