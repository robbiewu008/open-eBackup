rem @echo on
setlocal EnableDelayedExpansion
set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set PLUGIN_PATH=%CurBatPath%..\..\..\
set Ext_Pkg=%CurBatPath%..\..\..\Ext_pkg
echo PLUGIN_PATH is %PLUGIN_PATH%
if "%Module_Branch%" == "" (
    set Module_Branch=master_OceanProtect_DataBackup_1.6.0_smoke
)
rem create windows compile folder
cd "%CurBatPath%"
@REM if exist %PLUGIN_PATH%Windows_Compile  rd /S /Q "%PLUGIN_PATH%Windows_Compile"
IF not EXIST %PLUGIN_PATH%AppPlugins_NAS\Windows_Compile\plugins   MKDIR  %PLUGIN_PATH%AppPlugins_NAS\Windows_Compile\plugins
cd %PLUGIN_PATH%AppPlugins_NAS\framework\build\
call build.bat
cd "%CurBatPath%"
@REM pause
@REM rem Copy external pkg
@REM if exist %PLUGIN_PATH%Windows_Compile  rd /S /Q "%PLUGIN_PATH%Windows_Compile"
IF not EXIST %PLUGIN_PATH%Windows_Compile   MKDIR  %PLUGIN_PATH%Windows_Compile
xcopy /y /e %PLUGIN_PATH%AppPlugins_NAS\Windows_Compile  %PLUGIN_PATH%Windows_Compile
IF not EXIST %PLUGIN_PATH%Windows_Compile\plugins   MKDIR  %PLUGIN_PATH%Windows_Compile\plugins
xcopy /y /e %PLUGIN_PATH%\plugins\virtualization\vsprj  %PLUGIN_PATH%\Windows_Compile\vsprj\build
echo d | xcopy /y /e %PLUGIN_PATH%\plugins\virtualization  %PLUGIN_PATH%\Windows_Compile\plugins\virtualization


@REM call %PLUGIN_PATH%\Windows_Compile\Module\build\download_3rd.bat
call %PLUGIN_PATH%AppPlugins_NAS\Windows_Compile\Module\build\download_module_from_cmc.bat %Module_Branch%

echo d | xcopy /y /e %PLUGIN_PATH%AppPlugins_NAS\Windows_Compile\Module\libs %PLUGIN_PATH%\Windows_Compile\vsprj\bin\bin
cd "%PLUGIN_PATH%Windows_Compile%"
git-bash.exe -c "cd plugins/virtualization/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"

@REM copy zlib
IF not EXIST %PLUGIN_PATH%Windows_Compile\Module\open_src\zlib   MKDIR  %PLUGIN_PATH%Windows_Compile\Module\open_src\zlib
xcopy /y /e %Ext_Pkg%\zlib  %PLUGIN_PATH%\Windows_Compile\Module\open_src\zlib
COPY /Y %Ext_Pkg%\zlib\*.dll %PLUGIN_PATH%\Windows_Compile\vsprj\bin\bin
COPY /Y %Ext_Pkg%\zlib\*.lib %PLUGIN_PATH%\Windows_Compile\vsprj\bin\bin
rd /s /q  %Ext_Pkg%

set SOLUTION=%PLUGIN_PATH%Windows_Compile\vsprj\build\Virtualization.sln
IF not EXIST %SOLUTION%  call :FileNotFound

echo SOLUTION is %SOLUTION%

set BUILD_LOG="%PLUGIN_PATH%Windows_Compile\build.log"
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


rem Copy lib into plugins/file/lib
IF EXIST %PLUGIN_PATH%\plugins\virtualization\lib  rd /S /Q "%PLUGIN_PATH%\plugins\virtualization\lib"
IF not EXIST %PLUGIN_PATH%\plugins\virtualization\lib MKDIR  %PLUGIN_PATH%\plugins\virtualization\lib
COPY /Y %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.dll      %PLUGIN_PATH%\plugins\virtualization\lib
COPY /Y %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.pdb      %PLUGIN_PATH%\plugins\virtualization\lib

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
    type %PLUGIN_PATH%Windows_Compile\build.log
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
