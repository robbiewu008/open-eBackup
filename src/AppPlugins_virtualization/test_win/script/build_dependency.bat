set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set MODULE_PATH=%CurBatPath%..\..\..\..
echo MODULE_PATH is %MODULE_PATH%
set branch=%1
if "%branch%" == "" (
    set branch=develop_backup_software_1.5.0RC1
)
IF "%VIRT_ROOT_PATH%" == "" (set VIRT_ROOT_PATH=%~dp0..\..\..\..)

call :main
goto :eof

:main
    call %VIRT_ROOT_PATH%\plugins\virtualization\test_win\script\download_dependency.bat %branch%
    call %VIRT_ROOT_PATH%\Module\build\build_module.bat
    call %VIRT_ROOT_PATH%\framework\build\build.bat
    rem call %VIRT_ROOT_PATH%\Module\build\download_module_from_cmc.bat %branch%
    call :build_gtest
    call :copy_files
goto :eof

:build_gtest
    if exist %VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj rd /s /q %VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj
    if not exist %VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj mkdir %VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj
    cd %VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj
    cmake -G "Visual Studio 15 2017" -A x64 .. -D CMAKE_CXX_FLAGS_RELEASE=-MD -D CMAKE_CXX_FLAGS_DEBUG=-MD
    set GTEST_SLN=%VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj\googletest-distribution.sln
    echo GTEST_SLN is %GTEST_SLN%
    call %CurBatPath%\common_msvc_build.bat "%GTEST_SLN%"
goto :eof

:copy_files
    if not exist %VIRT_ROOT_PATH%\framework\inc mkdir %VIRT_ROOT_PATH%\framework\inc
    if not exist %VIRT_ROOT_PATH%\framework\lib mkdir %VIRT_ROOT_PATH%\framework\lib
    if not exist %VIRT_ROOT_PATH%\Module\libs mkdir %VIRT_ROOT_PATH%\Module\libs
    if not exist %VIRT_ROOT_PATH%\Module\open_src mkdir %VIRT_ROOT_PATH%\Module\open_src
    if not exist %VIRT_ROOT_PATH%\Module\platform mkdir %VIRT_ROOT_PATH%\Module\platform
    xcopy /s /e /y %VIRT_ROOT_PATH%\Windows_Compile\framework\inc\*  %VIRT_ROOT_PATH%\framework\inc
    xcopy /s /e /y %VIRT_ROOT_PATH%\Windows_Compile\vsprj\bin\bin\*  %VIRT_ROOT_PATH%\framework\lib
    xcopy /s /e /y %VIRT_ROOT_PATH%\Windows_Compile\Module\dt_utils\gmock\googletest-release\vsprj\lib\Release\* %VIRT_ROOT_PATH%\Module\libs
    xcopy /s /e /y %VIRT_ROOT_PATH%\Windows_Compile\Module\open_src  %VIRT_ROOT_PATH%\Module\open_src
    xcopy /s /e /y %VIRT_ROOT_PATH%\Windows_Compile\Module\platform  %VIRT_ROOT_PATH%\Module\platform
goto :eof