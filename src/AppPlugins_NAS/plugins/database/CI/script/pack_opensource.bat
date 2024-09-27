@echo off
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set PACK_LOG="%CurBatPath%CI_PACK.log"
set BASE_PATH=%CurBatPath%..\..\plugins\database\
echo BASE_PATH is %BASE_PATH% >> %PACK_LOG%
set APPLICATIONS_BUILD_PATH=%BASE_PATH%applications\build
set FRAMEWORK_PATH=%BASE_PATH%..\..\framework
set OUTPUT_PKG_PATH=%BASE_PATH%..\..\framework\output_pkg

set PLUGINS_ROOT_PATH=%CurBatPath%..\..\..\

rem open-source-obligation
set OPEN_OBLIGATION_ROOT_PATH=%CurBatPath%..\..\..\..\..\..\open-source-obligation
set OPEN_OBLIGATION_TRDPARTY_PATH=%OPEN_OBLIGATION_ROOT_PATH%\ThirdParty
set OPEN_OBLIGATION_PLUGIN_PATH=%OPEN_OBLIGATION_ROOT_PATH%\Plugin

set PYTHON_PLG_PKG_PATH=%PLUGINS_ROOT_PATH%\python3_pluginFrame
set MODULE_PKG_PATH=%OPEN_OBLIGATION_PLUGIN_PATH%\Module
set FS_SCANNER_PKG_PATH=%OPEN_OBLIGATION_PLUGIN_PATH%\FS_SCANNER
set FS_BACKUP_PKG_PATH=%OPEN_OBLIGATION_PLUGIN_PATH%\FS_BACKUP

set APP_GENERALDB_PKG_PATH=%OPEN_OBLIGATION_PLUGIN_PATH%\AppPlugins_GeneralDB\Windows

call :main
endlocal
goto :eof

:execute_build_script
    call %CurBatPath%build_opensource.bat
    if %errorlevel% NEQ 0 (
        echo Failed to execute the build script! >> %PACK_LOG%
        exit 1
    )
goto :eof

:create_dir
    ::create bin/applications path
    if not exist %OUTPUT_PKG_PATH%\bin\applications mkdir %OUTPUT_PKG_PATH%\bin\applications

    ::create conf path
    if not exist %OUTPUT_PKG_PATH%\conf mkdir %OUTPUT_PKG_PATH%\conf

    ::create service path
    if not exist %OUTPUT_PKG_PATH%\install mkdir %OUTPUT_PKG_PATH%\install
goto :eof

:download_python3_pluginFrame
    set python3_file=python3.pluginFrame.win32.zip
    if exist %OUTPUT_PKG_PATH%\install\%python3_file% (
        del %OUTPUT_PKG_PATH%\install\%python3_file%
    )

    xcopy /e/y %PYTHON_PLG_PKG_PATH%\%python3_file% %OUTPUT_PKG_PATH%\install
    if %errorlevel% NEQ 0 (
        echo Copy %python3_file% plugin error! >> %PACK_LOG%
        exit 1
    )
    echo "======================Copy python3 plugin package successfully=========================" >> %PACK_LOG%
goto :eof

:copy_file
    ::copy plugin_attribute json file
    if not exist %BASE_PATH%\conf\plugin_attribute_*.json (
        echo The plugin attribute json file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\conf\plugin_attribute_*.json %OUTPUT_PKG_PATH%\conf /e/y

    ::copy dll file
    if not exist %BASE_PATH%\lib\*.dll (
        echo The sqlserverPlugin exe file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\lib\*.dll %OUTPUT_PKG_PATH%\bin /e/y

    ::copy sqlserver API file
    if not exist %BASE_PATH%\src\applications\sqlserver\backupAPI\x64\Release\*.exe (
        echo The generaldb plugin dll file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\src\applications\sqlserver\backupAPI\x64\Release\*.exe %OUTPUT_PKG_PATH%\bin /e/y

    ::copy vssTool file
    if not exist %FRAMEWORK_PATH%\lib\VssTool.exe (
        echo VssTool file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %FRAMEWORK_PATH%\lib\VssTool.exe %OUTPUT_PKG_PATH%\bin /e/y

    ::copy other conf file
    xcopy %BASE_PATH%\conf\*.conf %OUTPUT_PKG_PATH%\conf /e/y

    ::copy hcpconf.ini
    if not exist %BASE_PATH%\conf\hcpconf.ini (
        echo The hcpconf.ini file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\conf\hcpconf.ini %OUTPUT_PKG_PATH%\conf /e/y

    ::copy install.bat
    if not exist %BASE_PATH%\install\install.bat (
        echo The install file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\install\install.bat %OUTPUT_PKG_PATH%\install /e/y

    ::copy start.bat
    if not exist %BASE_PATH%\install\start.bat (
        echo The start file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\install\start.bat %OUTPUT_PKG_PATH%\install /e/y

    ::copy python_env.bat
    if not exist %BASE_PATH%\build\python\python_env.bat (
        echo The python_env file cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\build\python\python_env.bat %OUTPUT_PKG_PATH%\install /e/y

    ::copy rpctool
    if not exist %BASE_PATH%\bin\rpctool.exe (
        echo The rpctool.exe cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\bin\rpctool.exe %OUTPUT_PKG_PATH%\bin /e/y
    if not exist %BASE_PATH%\src\tools\script\rpctool.bat (
        echo The rpctool.bat cannot be found! >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\src\tools\script\rpctool.bat %OUTPUT_PKG_PATH%\bin /e/y
goto :eof

:download_and_pack_scanner
    ::create lib path
    if not exist %BASE_PATH%\lib mkdir %BASE_PATH%\lib
    set scanner_file=Scanner.zip
    if exist %BASE_PATH%\lib\%scanner_file% (
        del %BASE_PATH%\lib\%scanner_file%
    )
    xcopy /e/y %FS_SCANNER_PKG_PATH%\Windows\%scanner_file% %BASE_PATH%\lib
    if %errorlevel% NEQ 0 (
        echo Download scanner from cmc error! >> %PACK_LOG%
        exit 1
    )
    echo "======================Copy scanner package successfully=========================" >> %PACK_LOG%
    7z.exe x %BASE_PATH%\lib\%scanner_file% -o%BASE_PATH%\lib
    if %errorlevel% NEQ 0 (
        echo "unzip scanner fail!" >> %PACK_LOG%
        exit 1
    )
    if not exist %BASE_PATH%\lib\Scanner\*.dll (
        echo "The Scanner dll file cannot be found!" >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\lib\Scanner\FS_Scanner.dll %OUTPUT_PKG_PATH%\bin /e/y
goto :eof

:download_and_pack_backup
    ::create lib path
    if not exist %BASE_PATH%\lib mkdir %BASE_PATH%\lib
    set fs_backup_file=FS_BACKUP.zip
    if exist %BASE_PATH%\lib\%fs_backup_file% (
        del %BASE_PATH%\lib\%fs_backup_file%
    )

    xcopy /e/y %FS_BACKUP_PKG_PATH%\Windows\%fs_backup_file% %BASE_PATH%\lib
    if %errorlevel% NEQ 0 (
        echo Copy fs backup from cmc error! >> %PACK_LOG%
        exit 1
    )
    echo "======================Copy fs backup package successfully=========================" >> %PACK_LOG%
    7z.exe x %BASE_PATH%\lib\%fs_backup_file% -o%BASE_PATH%\lib\FS_Backup\
    if %errorlevel% NEQ 0 (
        echo "unzip fs backup fail!" >> %PACK_LOG%
        exit 1
    )
    if not exist %BASE_PATH%\lib\FS_Backup\FS_Backup.dll (
        echo "The fs backup dll file cannot be found!" >> %PACK_LOG%
        exit 1
    )
    xcopy %BASE_PATH%\lib\FS_Backup\FS_Backup.dll %OUTPUT_PKG_PATH%\bin /e/y
goto :eof

:execute_app_build
    call %APPLICATIONS_BUILD_PATH%\build.bat %OUTPUT_PKG_PATH%\bin\applications %OUTPUT_PKG_PATH%\conf
    if %errorlevel% NEQ 0 (
        echo Failed to execute the application build script! >> %PACK_LOG%
        exit 1
    )
goto :eof

:copy_app_binary
    set script_dest_path=%OUTPUT_PKG_PATH%\bin\applications
    xcopy /e/y %APP_GENERALDB_PKG_PATH%\plugins\database\applications\. %script_dest_path%
    if %errorlevel% NEQ 0 (
        echo xcopy apps Failed  >> %BUILD_LOG%
        exit 1
    )
goto :eof

:execute_pack_script
    call %FRAMEWORK_PATH%\build\pack.bat
    if %errorlevel% NEQ 0 (
        echo Failed to execute framework pack script! >> %PACK_LOG%
        exit 1
    )
goto :eof

:main
    echo "#######################################################################################################"
    echo Start pack GeneralDbPlugin.
    echo Start pack GeneralDbPlugin. >> %PACK_LOG%
    echo "#######################################################################################################"

    call :execute_build_script

    call :create_dir

    call :download_python3_pluginFrame

    call :copy_file

    call :download_and_pack_scanner

    call :download_and_pack_backup

    call :execute_app_build

    call :copy_app_binary

    call :execute_pack_script

    echo "#######################################################################################################"
    echo GeneralDbPlugin pack success.
    echo GeneralDbPlugin pack success. >> %PACK_LOG%
    echo "#######################################################################################################"
goto :eof