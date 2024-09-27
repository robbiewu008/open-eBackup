set BUILD_LOG=%cd%\msvc_build.log
set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

set VCVARS32_BAT="%VCTOOL_HOME%\VC\Auxiliary\Build\vcvars32.bat"
echo VCVARS32_BAT is %VCVARS32_BAT% >> %BUILD_LOG%

set VCVARSX86_AMD64_BAT=%VCTOOL_HOME%\VC\Auxiliary\Build\vcvarsx86_amd64.bat
echo VCVARSX86_AMD64_BAT is %VCVARSX86_AMD64_BAT% >> %BUILD_LOG%

set DEVEVN_FILE=%VCTOOL_HOME%\Common7\IDE\devenv.exe
echo DEVEVN_FILE is %DEVEVN_FILE% >> %BUILD_LOG%

echo 1 is "%1"
set SLN_FILE=%1
echo SLN_FILE is "%1"
if %SLN_FILE% == "" (
    echo SLN_FILE can not be empty
    goto :eof
)
set build_type=%2
call :Compile64PluginFramework
goto :eof

:Compile32PluginFramework
	rem begin to make openssl 32 lib file
	rem setting the enviroment with VC command 32
	set SOLUTION_CONFIG="Release|Win32"
	set ACTION=Rebuild
	call %VCVARS32_BAT%

	%Drive%
	cd %CurBatPath%

	echo Begin to clean compile >> %BUILD_LOG%
	echo %DEVEVN_FILE% "%SLN_FILE%" /clean
	%DEVEVN_FILE% "%SLN_FILE%" /clean
    if "%build_type%" == "TracePoint" (
        set _CL_=/DEBK_TRACE_POINT
        set _LINK_="tracepoint.lib"
    )
	echo Begin to rebuild the Plugin >> %BUILD_LOG%
	echo %DEVEVN_FILE% %SLN_FILE% /%ACTION% %SOLUTION_CONFIG% /useenv
	%DEVEVN_FILE% %SLN_FILE% /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

	echo End to build x32 Agent
goto :eof

:Compile64PluginFramework
    %Drive%
    rem build src
    cd %CurBatPath%
    set SOLUTION_CONFIG="Release|x64"
    set ACTION=Rebuild
	call %VCVARSX86_AMD64_BAT%
    echo Begin to clean compile >> %BUILD_LOG%
    echo %DEVEVN_FILE% "%SLN_FILE%" /clean
    %DEVEVN_FILE% %SLN_FILE% /clean
    if "%build_type%" == "TracePoint" (
        set _CL_=/DEBK_TRACE_POINT
        set _LINK_="tracepoint.lib"
    )
    echo Begin to rebuild the Plugin >> %BUILD_LOG%
    echo %DEVEVN_FILE% %SLN_FILE% /%ACTION% %SOLUTION_CONFIG% /useenv
    %DEVEVN_FILE% %SLN_FILE% /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

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
