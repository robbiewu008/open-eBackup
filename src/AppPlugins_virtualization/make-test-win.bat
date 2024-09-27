set TEST_CUR_BAT_PATH=%~dp0
set VIRT_ROOT_PATH=%TEST_CUR_BAT_PATH%..\..
echo VIRT_ROOT_PATH is %VIRT_ROOT_PATH%
set PLUGIN_FRAMEWORK_LIB_LIBEVENT=%VIRT_ROOT_PATH%\Module\third_open_src\libevent_rel\lib

set build_type=%1

call :copy_test_src
call :msvc_llt_build_test
goto :eof

@rem function definitions
:msvc_llt_build_test
    cd "%TEST_CUR_BAT_PATH%
    git-bash.exe -c "cd test_win;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"
    git-bash.exe -c "cd src;find -name *.cpp | xargs unix2dos; find -name *.h | xargs unix2dos;"
    call %TEST_CUR_BAT_PATH%test_win\script\build_dependency.bat
    call :vcprj_build

    xcopy /s /e /y %VIRT_ROOT_PATH%\Module\libs\*.dll %VIRT_ROOT_PATH%\plugins\virtualization\test_win\vsprj\x64\Release
    xcopy /s /e /y %VIRT_ROOT_PATH%\framework\lib\*.dll %VIRT_ROOT_PATH%\plugins\virtualization\test_win\vsprj\x64\Release
    cd %VIRT_ROOT_PATH%\plugins\virtualization\test_win\vsprj\x64\Release
    @rem execute test case
    .\virt_hyperv_test.exe
    @rem generate coverage report
    OpenCppCoverage --sources %TEST_CUR_BAT_PATH%src\protect_engines\hyperv --sources %TEST_CUR_BAT_PATH%src\volume_handlers\hyperv_volume --sources %TEST_CUR_BAT_PATH%src\repository_handlers\win32filesystem -- virt_hyperv_test.exe
goto :eof

:vcprj_build
    @rem echo begin create visual studio project by Cmake...
    @rem if exist %TEST_CUR_BAT_PATH%test_win\vsprj rd /s /q %TEST_CUR_BAT_PATH%test_win\vsprj
    @rem if not exist %TEST_CUR_BAT_PATH%test_win\vsprj mkdir %TEST_CUR_BAT_PATH%test_win\vsprj
    @rem cd %TEST_CUR_BAT_PATH%test_win\vsprj
    @rem cmake -G "Visual Studio 15 2017" -A x64 ..
    set VIRT_TEST_SLN=%TEST_CUR_BAT_PATH%\test_win\vsprj\virtualization_plugin_test.sln
    call %TEST_CUR_BAT_PATH%test_win\script\common_msvc_build.bat "%VIRT_TEST_SLN%" %build_type%
goto :eof

:hdt_test
    set action=%1
    if "%action%"=="clean" (hdt clean ./test_win)
    if EXIST %TEST_CUR_BAT_PATH%test_win\build (hdt clean ./test_win)
    if not exist %TEST_CUR_BAT_PATH%test_win\src     mkdir %TEST_CUR_BAT_PATH%test_win\src
    xcopy /s /e /y %TEST_CUR_BAT_PATH%src\* %TEST_CUR_BAT_PATH%test_win\src\

    cd %TEST_CUR_BAT_PATH%
    hdt test ./test_win -p %PLUGIN_FRAMEWORK_LIB_LIBEVENT% -c on -s off --args="--gtest_output=xml:report.xml"
    hdt report ./test_win --args="--exclude-unreachable-branches --exclude-throw-branches --filter=../src/.*\.
goto :eof

:copy_test_src
    if not exist %TEST_CUR_BAT_PATH%test_win\common    mkdir %TEST_CUR_BAT_PATH%test_win\common
    if not exist %TEST_CUR_BAT_PATH%test_win\conf    mkdir %TEST_CUR_BAT_PATH%test_win\conf
    if not exist %TEST_CUR_BAT_PATH%test_win\dt_fuzz    mkdir %TEST_CUR_BAT_PATH%test_win\dt_fuzz
    if not exist %TEST_CUR_BAT_PATH%test_win\job_controller    mkdir %TEST_CUR_BAT_PATH%test_win\job_controller
    if not exist %TEST_CUR_BAT_PATH%test_win\llt_stub    mkdir %TEST_CUR_BAT_PATH%test_win\llt_stub
    if not exist %TEST_CUR_BAT_PATH%test_win\protect_engines    mkdir %TEST_CUR_BAT_PATH%test_win\protect_engines
    if not exist %TEST_CUR_BAT_PATH%test_win\repository_handlers    mkdir %TEST_CUR_BAT_PATH%test_win\repository_handlers
    if not exist %TEST_CUR_BAT_PATH%test_win\volume_handlers    mkdir %TEST_CUR_BAT_PATH%test_win\volume_handlers
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\common\*    %TEST_CUR_BAT_PATH%test_win\common\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\conf\*    %TEST_CUR_BAT_PATH%test_win\conf\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\dt_fuzz\*    %TEST_CUR_BAT_PATH%test_win\dt_fuzz\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\job_controller\*    %TEST_CUR_BAT_PATH%test_win\job_controller\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\llt_stub\*    %TEST_CUR_BAT_PATH%test_win\llt_stub\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\protect_engines\*    %TEST_CUR_BAT_PATH%test_win\protect_engines\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\repository_handlers\*    %TEST_CUR_BAT_PATH%test_win\repository_handlers\
    xcopy /s /e /y %TEST_CUR_BAT_PATH%test\volume_handlers\*    %TEST_CUR_BAT_PATH%test_win\volume_handlers\
goto :eof