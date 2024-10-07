set VIRT_ROOT_PATH=%~dp0..\..\..\..
set branch=%1
if "%branch%" == "" (set branch=develop_backup_software_1.5.0RC1)

call :main
goto :eof

:main
    call:downloadFramework
    call:downloadModule
goto :eof

:downloadFramework
    git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectagent/AppPlugins_NAS.git -b %branch%
    IF not EXIST %VIRT_ROOT_PATH%\framework (
        MKDIR %VIRT_ROOT_PATH%\framework
        xcopy /s /e /y .\AppPlugins_NAS\framework %VIRT_ROOT_PATH%\framework
    ) else (
        echo framework folder exists, if you want to download latest please remove it.
    ) 
    IF not EXIST %VIRT_ROOT_PATH%\vsprj (
        MKDIR %VIRT_ROOT_PATH%\vsprj
        xcopy /s /e /y .\AppPlugins_NAS\vsprj %VIRT_ROOT_PATH%\vsprj
    ) else (
        echo vsprj folder exists, if you want to download latest please remove it.
    )     
    rd /s /q AppPlugins_NAS
goto:eof

:downloadModule
    git clone ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git -b %branch%
    IF not EXIST %VIRT_ROOT_PATH%\Module (
        MKDIR %VIRT_ROOT_PATH%\Module
        xcopy /s /e /y .\Module %VIRT_ROOT_PATH%\Module
    ) else (
        echo Module folder exists, if you want to download latest please remove it.
    )
    rd /s /q Module
    cd %VIRT_ROOT_PATH%\Module\build
goto :eof
