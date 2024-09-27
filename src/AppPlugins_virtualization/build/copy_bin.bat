rem @echo on
setlocal EnableDelayedExpansion
set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set PLUGIN_PATH=%CurBatPath%..\..\..\
echo PLUGIN_PATH is %PLUGIN_PATH%
set Ext_Pkg=%CurBatPath%..\..\..\Ext_pkg
set FRAMEWORK_PATH=%PLUGIN_PATH%AppPlugins_NAS\framework

if "%1" EQU "" (
    echo "Target path is missing"
    exit 1
) else (
    set TARGET_BIN_PATH=%1
    echo TARGET_BIN_PATH is %TARGET_BIN_PATH%
)

if "%2" EQU "" (
   set BRANCH=master_OceanProtect_DataBackup
) else (
   set BRANCH=%2
   echo BRANCH is %BRANCH%
)

set ModuleIncludeList=src platform\securec\include open_src\jsoncpp\include open_src\boost\.libs\include open_src\thrift\lib\cpp\src ^
open_src\tinyxml2\include open_src\zlib open_src\openssl\include third_open_src\curl_rel\include third_open_src\lz4_rel\include ^
third_open_src\libaio_rel\include

set frameworkIncludeList=src inc dep\agent_sdk\include dep\agent_sdk\include\message\archivestream

call :main
endlocal
goto :eof

:Handle3rdDependency
    echo Handle3rdDependency
    call %CurBatPath%download_build_third.bat %BRANCH%
    set res=%errorlevel%
        if %res% NEQ 0 (
        echo download_build_third.bat failed, pls check
        exit 1
    )

goto :eof

:CompileOpensrcDependcy
    echo CompileOpensrcDependcy
    @REM Replace sln file
    if exist %CurBatPath%..\vsprj\OpensourceDependency.sln (
        copy /y %CurBatPath%..\vsprj\OpensourceDependency.sln %CurBatPath%..\vsprj\Virtualization.sln
    ) else (
        echo No opensource dependency sln provided, pls check
        exit 1    
    )

    call %CurBatPath%build.bat
    set res=%errorlevel%
        if %res% NEQ 0 (
        echo compile error, pls check
        exit 1
    )
goto :eof

:CopyAdditionalIncludeFiles
    echo CopyAdditionalIncludeFiles

    rem Copy Module include
    for %%i in (%ModuleIncludeList%) do (
        xcopy /y /e /i %PLUGIN_PATH%\Windows_Compile\Module\%%i %PLUGIN_PATH%\tmp\Module\%%i
    )
    
    rem Copy framework include
    for %%i in (%frameworkIncludeList%) do (
        xcopy /y /e /i %PLUGIN_PATH%\Windows_Compile\framework\%%i %PLUGIN_PATH%\tmp\framework\%%i
    )

goto :eof

:CopyDependentbin
    echo CopyDependentbin

    rem Copy vsprj bin
    xcopy /y /e /i %PLUGIN_PATH%\Windows_Compile\vsprj\bin %PLUGIN_PATH%\tmp\vsprj\bin


goto :eof

:CopyPackageFiles
    rem Reset tmp dir
    IF EXIST %PLUGIN_PATH%\tmp  rd /S /Q "%PLUGIN_PATH%\tmp"
    IF not EXIST %PLUGIN_PATH%\tmp MKDIR  %PLUGIN_PATH%\tmp

    rem Reset win_pkg dir
    IF EXIST %TARGET_BIN_PATH%\AppPlugins_virtualization\win_pkg  rd /S /Q "%TARGET_BIN_PATH%\AppPlugins_virtualization\win_pkg"
    IF not EXIST %TARGET_BIN_PATH%\AppPlugins_virtualization\win_pkg  MKDIR  %TARGET_BIN_PATH%\AppPlugins_virtualization\win_pkg 

    rem Copy conf for package
    xcopy /y /e /i %PLUGIN_PATH%\conf %TARGET_BIN_PATH%\AppPlugins_virtualization\win_pkg\conf

    rem Copy framewokr package files
    xcopy /y /e /i %FRAMEWORK_PATH%\build\pack.bat %PLUGIN_PATH%\tmp\framework\build\
    xcopy /y /e /i %FRAMEWORK_PATH%\lib %PLUGIN_PATH%\tmp\framework\lib
    xcopy /y /e /i %FRAMEWORK_PATH%\vc_redist %PLUGIN_PATH%\tmp\framework\vc_redist
    xcopy /y /e /i %FRAMEWORK_PATH%\conf %PLUGIN_PATH%\tmp\framework\conf
    xcopy /y /e /i %FRAMEWORK_PATH%\build\install %PLUGIN_PATH%\tmp\framework\build\install

goto :eof

:CopyBin
    echo CopyBin

    call :CopyAdditionalIncludeFiles

    call :CopyDependentbin
    
    cd %CurBatPath%
    7z a Open_src_dependency.zip %PLUGIN_PATH%\tmp\* -r

    move "Open_src_dependency.zip" "%TARGET_BIN_PATH%\AppPlugins_virtualization\win_pkg "

goto :eof

:main
    echo "#######################################################################################################"
    echo Start copy VirtualizationPlugin bin.
    echo "#######################################################################################################"

    call :Handle3rdDependency

    call :CompileOpensrcDependcy

    call :CopyPackageFiles

    call :CopyBin

    echo "#######################################################################################################"
    echo Copy VirtualizationPlugin bin success.
    echo "#######################################################################################################"
goto :eof

goto :eof
     echo "Finish"
     Exit