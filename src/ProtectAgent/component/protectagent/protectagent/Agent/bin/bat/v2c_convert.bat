@echo off

rem ***************************************************************************************
rem program name:          v2c_convert.bat     
rem function:              convert vmare sys volume to boot in kvm environment
rem author:                d00418490
rem time:                  2019-5-21
rem function and description:  
rem author:
rem time:
rem explain:
rem ***************************************************************************************

set DISKPART_RESULT=C:\tmp_result
set UserSysFullVersion=0
set OsVersion=0
set UserSysFullProduct=0
set OsProduct=0
set UserSysFullProcessorArch=0
set ArchType=0
set DriverName=0
set OrginArchTye=0
set SYS_FOLDER=\Windows 
set SYS_LETTER=0
set DriverExist=0
set EXIT_CODE=0

set ERROR_SCRIPT_WINDOWS_V2C_RESCAN_DISK_FAILED=211
set ERROR_SCRIPT_WINDOWS_V2C_DISK_LETTER_NOT_IN_RANGE=212
set ERROR_SCRIPT_WINDOWS_V2C_REGLOAD_FAILED=213
set ERROR_SCRIPT_WINDOWS_V2C_VERSION_UNSUPPORTED=214
set ERROR_SCRIPT_WINDOWS_V2C_INSTALL_DRIVER_FAILED=215

::online_disk
(
echo rescan
echo list disk
echo select disk 1
echo online disk
echo attributes disk clear readonly
echo list partition
echo select partition 2
echo detail partition
echo exit
) | diskpart > %DISKPART_RESULT%
if not %errorlevel% == 0 ( goto ErrorExitRescan )
echo %DISKPART_RESULT%

::get_user_sys_disk_letter
for /f "tokens=4" %%a in (' type %DISKPART_RESULT%^|find /i "Healthy"') do ( set SYS_LETTER=%%a )
echo "before SYS_LETTER is" %SYS_LETTER%
set SYS_LETTER=%SYS_LETTER:~0,1%
echo "after SYS_LETTER is" %SYS_LETTER%
if %SYS_LETTER% == 0 ( goto ErrorExitDiskLetter )

::load_user_registery_and_get_sys_version
reg load HKLM\OFFLINE_SOFTWARE %SYS_LETTER%:\Windows\System32\config\Software
if not %errorlevel% == 0 ( goto ErrorExitRegload )
set cmdVersion=reg query "HKLM\OFFLINE_SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v CurrentVersion
for /F "tokens=*" %%i in (' %cmdVersion% ') do (set UserSysFullVersion=%%i)
set cmdProduct=reg query "HKLM\OFFLINE_SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v ProductName
for /F "tokens=*" %%i in (' %cmdProduct% ') do (set UserSysFullProduct=%%i)
reg unload HKLM\OFFLINE_SOFTWARE

reg load HKLM\OFFLINE_SYSTEM %SYS_LETTER%:\Windows\System32\config\System
if not %errorlevel% == 0 ( goto ErrorExitRegload )
set cmdArch=reg query "HKLM\OFFLINE_SYSTEM\ControlSet001\Control\Session Manager\Environment" /v PROCESSOR_ARCHITECTURE
for /F "tokens=*" %%i in (' %cmdArch% ') do (set UserSysFullProcessorArch=%%i)
reg unload HKLM\OFFLINE_SYSTEM

for /f "tokens=3" %%i in ("%UserSysFullProcessorArch%") do ( set OrginArchTye=%%i )
if %OrginArchTye%==x86 (set "ArchType=x86") else set "ArchType=amd64"
echo %UserSysFullProduct%
echo %UserSysFullVersion%
echo %UserSysFullProcessorArch%
echo %OrginArchTye%
echo %ArchType%

::define_driver_version
echo "%UserSysFullProduct%" | find "2008 R2"
if %errorlevel% == 0 ( 
    echo "%UserSysFullVersion%" | find "6.1"
    if %errorlevel% == 0 (
        goto Server2008R2
    )
)
echo "%UserSysFullProduct%" | find "2012"
if %errorlevel% == 0 ( 
    echo "%UserSysFullVersion%" | find "6.2"
    if %errorlevel% == 0 (
        goto Server2012
    )
)
echo "%UserSysFullProduct%" | find "2012 R2"
if %errorlevel% == 0 ( 
    echo "%UserSysFullVersion%" | find "6.3"
    if %errorlevel% == 0 (
        goto Server2012R2
    )
)
echo "%UserSysFullProduct%" | find "2016"
if %errorlevel% == 0 ( 
    echo "%UserSysFullVersion%" | find "6.3"
    if %errorlevel% == 0 (
        goto Server2016
    )
)
goto UnknownVersion

:Server2008R2
set OsVersion=Windows Server 2008 R2
set DriverName=2k8R2
goto Show

:Server2012
set OsVersion=Windows Server 2012
set DriverName=2k12
goto Show

:Server2012R2
set OsVersion=Windows Server 2012 R2
set DriverName=2k12R2
goto Show

:Server2016
set OsVersion=Windows Server 2016
set DriverName=2k16
goto Show

:UnknownVersion
echo %UserSysFullProduct% 
echo %UserSysFullVersion%
goto ErrorExitVersion

:Show 
echo %OsVersion% 
echo %DriverName%
echo %ArchType%

::check_if_driver_installed
set balloonExist=1
set netkvmExist=1
set vioscsiExist=1
set vioserExist=1
set viostorExist=1

dism.exe /image:%SYS_LETTER%:\ /get-drivers | findstr balloon 
set balloonExist=%errorlevel%
dism.exe /image:%SYS_LETTER%:\ /get-drivers | findstr netkvm 
set netkvmExist=%errorlevel%
dism.exe /image:%SYS_LETTER%:\ /get-drivers | findstr vioscsi 
set vioscsiExist=%errorlevel%
dism.exe /image:%SYS_LETTER%:\ /get-drivers | findstr vioser 
set vioserExist=%errorlevel%
dism.exe /image:%SYS_LETTER%:\ /get-drivers | findstr viostor 
set viostorExist=%errorlevel%

::install_driver
if %balloonExist% == 1 (
dism.exe /image:%SYS_LETTER%:\ /add-driver /driver:C:\windows_kvm_driver\Balloon\%DriverName%\%ArchType%\balloon.inf )
if %netkvmExist% == 1 (
dism.exe /image:%SYS_LETTER%:\ /add-driver /driver:C:\windows_kvm_driver\NetKVM\%DriverName%\%ArchType%\netkvm.inf )
if %vioscsiExist% == 1 (
dism.exe /image:%SYS_LETTER%:\ /add-driver /driver:C:\windows_kvm_driver\vioscsi\%DriverName%\%ArchType%\vioscsi.inf )
if %vioserExist% == 1 (
dism.exe /image:%SYS_LETTER%:\ /add-driver /driver:C:\windows_kvm_driver\vioserial\%DriverName%\%ArchType%\vioser.inf )
if %viostorExist% == 1 (
dism.exe /image:%SYS_LETTER%:\ /add-driver /driver:C:\windows_kvm_driver\viostor\%DriverName%\%ArchType%\viostor.inf )  

if not %errorlevel% == 0 ( goto ErrorExitDriver )
goto NormalExit

:NormalExit
set EXIT_CODE=0
goto OfflineDiskAndExit

:ErrorExitRescan
set EXIT_CODE=%ERROR_SCRIPT_WINDOWS_V2C_RESCAN_DISK_FAILED%
goto OfflineDiskAndExit

:ErrorExitDiskLetter
set EXIT_CODE=%ERROR_SCRIPT_WINDOWS_V2C_DISK_LETTER_NOT_IN_RANGE%
goto OfflineDiskAndExit

:ErrorExitRegload
set EXIT_CODE=%ERROR_SCRIPT_WINDOWS_V2C_REGLOAD_FAILED%
goto OfflineDiskAndExit

:ErrorExitVersion
set EXIT_CODE=%ERROR_SCRIPT_WINDOWS_V2C_VERSION_UNSUPPORTED%
goto OfflineDiskAndExit

:ErrorExitDriver
set EXIT_CODE=%ERROR_SCRIPT_WINDOWS_V2C_INSTALL_DRIVER_FAILED%
goto OfflineDiskAndExit


:OfflineDiskAndExit
(
echo rescan
echo list disk
echo select disk 1
echo offline disk
echo exit
) | diskpart
del %DISKPART_RESULT%
exit /b %EXIT_CODE%














