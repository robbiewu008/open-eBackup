rem @echo off
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set ModuleRootPath=%CurBatPath%..\
echo ModuleRootPath is %ModuleRootPath%

IF exist %ModuleRootPath%\EXT_Pkg  rd /S /Q %ModuleRootPath%\EXT_Pkg
IF not exist %ModuleRootPath%\EXT_Pkg  mkdir %ModuleRootPath%\EXT_Pkg

artget.exe pull -d %CurBatPath%LCRP\conf\code_from_cmc.xml -p "{'componentVersion':'1.1.0','PRODUCT':'dorado', 'CODE_BRANCH':'debug_OceanProtect_DataBackup_1.6.0_securecheck','COMPONENT_TYPE':'Plugins/Windows','ARCH':'framework_dep_pkg/opensrc_platform.zip'}" -ap %ModuleRootPath%\EXT_Pkg -user %cmc_user% -pwd %cmc_pwd%

artget.exe pull "OceanProtect_Opensource_3rdparty_Centralized 1.2.1RC1" -ru software -user %cmc_user% -pwd %cmc_pwd% -rp "master_OceanProtect_DataBackup_1.6.0_smoke/Windows2012/x64/third_party_groupware/chksgfiles.zip" -ap "%ModuleRootPath%\EXT_Pkg"

IF not EXIST %ModuleRootPath%\open_src\chksgfiles   MKDIR  %ModuleRootPath%\open_src\chksgfiles

7z.exe x -y "%ModuleRootPath%\EXT_Pkg\opensrc_platform.zip" -o%ModuleRootPath% >nul
7z.exe x -y "%ModuleRootPath%\EXT_Pkg\chksgfiles.zip" -o%ModuleRootPath%\open_src\chksgfiles >nul

IF not EXIST %ModuleRootPath%\open_src\all_libs   MKDIR  %ModuleRootPath%\open_src\all_libs

xcopy /y /e %ModuleRootPath%\all_libs   %ModuleRootPath%\open_src\all_libs
xcopy /y /e %ModuleRootPath%\open_src\chksgfiles\chksgfiles.lib   %ModuleRootPath%\open_src\all_libs
xcopy /y /e %ModuleRootPath%\open_src\chksgfiles\ese.dll   %ModuleRootPath%\open_src\all_libs
xcopy /y /e %ModuleRootPath%\open_src\chksgfiles\chksgfiles.dll   %ModuleRootPath%\open_src\all_libs

IF exist %ModuleRootPath%\EXT_Pkg  rd /S /Q %ModuleRootPath%\EXT_Pkg

rem vs2017 has bug not support the alias of fuction pointer,so need to replace it.
git-bash.exe -c "sed -i 's/using signature = void(BOOST_PROCESS_V2_NAMESPACE::error_code);/typedef void(\*signature)(BOOST_PROCESS_V2_NAMESPACE::error_code);/g' ../../Windows_Compile/Module/open_src/boost/.libs/include/boost/process/v2/exit_code.hpp"

endlocal
