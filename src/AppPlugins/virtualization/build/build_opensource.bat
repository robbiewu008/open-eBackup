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
rem @echo on
setlocal EnableDelayedExpansion
set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set PACK_LOG="%CurBatPath%CI_PACK.log"

set PLUGIN_PATH=%CurBatPath%..\..\..\
set Ext_Pkg=%CurBatPath%..\..\..\Ext_pkg
set FRAMEWORK_PATH=%PLUGIN_PATH%Windows_Compile\framework
set OUTPUT_PKG_PATH=%FRAMEWORK_PATH%\output_pkg
set FINAL_PKG_PATH=%BASE_PATH%\output_pkg
echo PLUGIN_PATH is %PLUGIN_PATH%

if "%1" EQU "" (
    echo "Bin repository path is missing"
    exit 1
) else (
    set BIN_REPO_PATH=%1
    echo BIN_REPO_PATH is %BIN_REPO_PATH%
)
set BIN_PKG_PATH=%BIN_REPO_PATH%\AppPlugins_virtualization\win_pkg

call :main
endlocal
goto :eof

:GetDependency

    IF not EXIST %BIN_PKG_PATH%\Open_src_dependency.zip (
        echo "No %BIN_PKG_PATH%\Open_src_dependency.zip provided"
        exit 1
    )
    7z.exe x -y "%BIN_PKG_PATH%\Open_src_dependency.zip" -o%PLUGIN_PATH%Windows_Compile >nul
goto :eof

:PackageBin
    call :create_dir

    call :copy_file

    call :execute_pack_script
goto :eof

:create_dir
    ::create bin/applications path
    if not exist %OUTPUT_PKG_PATH%\bin\applications mkdir %OUTPUT_PKG_PATH%\bin\applications

    :: create pdb for debug
    if not exist %OUTPUT_PKG_PATH%\pdb mkdir %OUTPUT_PKG_PATH%\pdb

    ::create conf path
    if not exist %OUTPUT_PKG_PATH%\conf mkdir %OUTPUT_PKG_PATH%\conf

    ::create service path
    if not exist %OUTPUT_PKG_PATH%\install mkdir %OUTPUT_PKG_PATH%\install

    if not exist %FINAL_PKG_PATH% mkdir %FINAL_PKG_PATH%
goto :eof

:copy_file
    ::check framework
    if not exist %FRAMEWORK_PATH% (
        echo The framework dir cannot be found! >> %PACK_LOG%
        exit 1
    )

    ::copy plugin_attribute json file
    if not exist %BIN_PKG_PATH%\conf\plugin_attribute_*.json (
        echo %BIN_PKG_PATH%\conf\
        pause
        echo The plugin attribute json file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BIN_PKG_PATH%\conf\plugin_attribute_*.json %OUTPUT_PKG_PATH%\conf /e/y

    ::copy dll file
    if not exist %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.dll (
        echo The FilePlugin exe file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.dll %OUTPUT_PKG_PATH%\bin /e/y

    xcopy %PLUGIN_PATH%Windows_Compile\vsprj\bin\bin\*.pdb %OUTPUT_PKG_PATH%\pdb /e/y
    ::copy powershell
    xcopy %PLUGIN_PATH%plugins\virtualization\src\protect_engines\hyperv\api\powershell\*.ps1 %OUTPUT_PKG_PATH%\bin /e/y
    ::copy other conf file
    xcopy %BIN_PKG_PATH%\conf\*.conf %OUTPUT_PKG_PATH%\conf /e/y

    ::copy hcpconf.ini
    if not exist %BIN_PKG_PATH%\conf\hcpconf.ini (
        echo The hcpconf.ini file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BIN_PKG_PATH%\conf\hcpconf.ini %OUTPUT_PKG_PATH%\conf /e/y

    ::copy install.bat
    if not exist %PLUGIN_PATH%plugins\virtualization\install\install.bat (
        echo The install file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %PLUGIN_PATH%plugins\virtualization\install\install.bat %OUTPUT_PKG_PATH%\install /e/y
    
    ::copy start.bat
    if not exist %PLUGIN_PATH%plugins\virtualization\install\start.bat (
        echo The start file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %PLUGIN_PATH%plugins\virtualization\install\start.bat %OUTPUT_PKG_PATH%\install /e/y

goto :eof

:execute_pack_script
    call %FRAMEWORK_PATH%\build\pack.bat
    if %errorlevel% NEQ 0 (
        echo Failed to execute framework pack script! >> %PACK_LOG%
        exit 1
    )
    xcopy %OUTPUT_PKG_PATH% %FINAL_PKG_PATH% /e/y
goto :eof

:CompileBin
    rem create windows compile folder

    cd "%CurBatPath%"
    IF not EXIST %PLUGIN_PATH%Windows_Compile   MKDIR  %PLUGIN_PATH%Windows_Compile

    IF not EXIST %PLUGIN_PATH%Windows_Compile\plugins   MKDIR  %PLUGIN_PATH%Windows_Compile\plugins
    echo d | xcopy /y /e %PLUGIN_PATH%\plugins\virtualization\vsprj  %PLUGIN_PATH%\Windows_Compile\vsprj\build
    echo d | xcopy /y /e %PLUGIN_PATH%\plugins\virtualization  %PLUGIN_PATH%\Windows_Compile\plugins\virtualization

    cd "%PLUGIN_PATH%Windows_Compile%"
    git-bash.exe -c "cd plugins/virtualization/src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"

    call :SwitchVsprjFiles

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

:SwitchVsprjFiles
    echo SwitchVsprjFiles
    if exist %CurBatPath%..\vsprj\Virtualization_opensource.vcxproj.filters (
        copy /y %CurBatPath%..\vsprj\Virtualization_opensource.vcxproj.filters %CurBatPath%..\Virtualization.vcxproj.filters
    ) else (
        echo No Virtualization_opensource.vcxproj.filters provided, pls check
        exit 1    
    )
    if exist %CurBatPath%..\vsprj\Virtualization_opensource.vcxproj (
        copy /y %CurBatPath%..\vsprj\Virtualization_opensource.vcxproj %CurBatPath%..\vsprj\Virtualization.vcxproj
    ) else (
        echo No Virtualization_opensource.vcxproj provided, pls check
        exit 1    
    )
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
        @REM exit 1    
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

:main
    echo "#######################################################################################################"
    echo Start build opensource VirtualizationPlugin pkg.
    echo "#######################################################################################################"

    call :GetDependency

    call :CompileBin

    call :PackageBin

    echo "#######################################################################################################"
    echo Build opensource VirtualizationPlugin pkg success.
    echo "#######################################################################################################"
goto :eof

goto :eof
     echo "Finish"
     Exit
