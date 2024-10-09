::Create python virtual env for DB plugin
@echo off
setlocal enableextensions

set Drive=%~d0
set CurBatPath=%~dp0
set ENV_LOG=%CurBatPath%\..\..\..\ProtectClient-E\log\Plugins\GeneralDBPlugin\python_env.log
set PACKAGE_PATH=%CurBatPath%\packages_required
set INITIAL_PATH=%CurBatPath%\initial_packages
set PYTHON_PATH=%CurBatPath%Python310
set PYTHON3=%PYTHON_PATH%\python.exe
set Path=%PYTHON_PATH%\;%PYTHON_PATH%\Scripts\;%Path%
set WORKON_HOME=%CurBatPath%virtualenvs

call :main
endlocal
goto :eof

:virtualenvwrapper
    cd /d %INITIAL_PATH%
    %PYTHON3% -m pip install --no-index --find-links=./ -r initial-win.txt >> %ENV_LOG%
    if %errorlevel% NEQ 0 (
        echo Failed to install virtualenvwrapper ! >> %ENV_LOG%
        exit /b 1
    )
goto :eof

:create_env
    if exist %CurBatPath%virtualenvs (
        echo %CurBatPath%virtualenvs exist, will rm. >> %ENV_LOG%
        rmdir /s/q %CurBatPath%\virtualenvs
    )
    mkdir %CurBatPath%virtualenvs

    cd /d "%PYTHON_PATH%\Scripts"
    call mkvirtualenv.bat plugin_env >> %ENV_LOG%
    if %errorlevel% NEQ 0 (
        echo Failed to mkvirtualenv ! >> %ENV_LOG%
        exit /b 1
    )
    echo mkvirtualenv success. >> %ENV_LOG%

    cd /d "%PACKAGE_PATH%"
    python -m pip install --no-index --find-links=./ -r requirements-win.txt >> %ENV_LOG%
    echo Create plugin virtualenv success. >> %ENV_LOG%
goto :eof

:main
    %PYTHON3% -m pip config set global.index-url http://mirrors.tools.huawei.com/pypi/simple >> %ENV_LOG%
    if %errorlevel% NEQ 0 (
        echo Failed to set global.index-url for pip ! >> %ENV_LOG%
        exit /b 1
    )
    call :virtualenvwrapper
    call :create_env
    rd /s/q %CurBatPath%\packages_required
    rd /s/q %CurBatPath%\initial_packages
goto :eof