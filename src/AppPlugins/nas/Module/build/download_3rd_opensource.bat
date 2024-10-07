rem @echo off
setlocal EnableDelayedExpansion
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%
set ModuleRootPath=%CurBatPath%..\
echo ModuleRootPath is %ModuleRootPath%
set open_source_obligation_path=%CurBatPath%..\..\..\open-source-obligation
set opensrc_platform_zip=%open_source_obligation_path%\ThirdParty\Windows2012\x64\third_party_groupware\opensrc_platform.zip
set chksgfiles_zip=%open_source_obligation_path%\ThirdParty\Windows2012\x64\third_party_groupware\chksgfiles.zip

IF exist %ModuleRootPath%\EXT_Pkg  rd /S /Q %ModuleRootPath%\EXT_Pkg
IF not exist %ModuleRootPath%\EXT_Pkg  mkdir %ModuleRootPath%\EXT_Pkg

IF not EXIST %ModuleRootPath%\open_src\chksgfiles   MKDIR  %ModuleRootPath%\open_src\chksgfiles

7z.exe x -y "%opensrc_platform_zip%" -o%ModuleRootPath% >nul
7z.exe x -y "%chksgfiles_zip%" -o%ModuleRootPath%\open_src\chksgfiles >nul

IF not EXIST %ModuleRootPath%\open_src\all_libs   MKDIR  %ModuleRootPath%\open_src\all_libs

xcopy /y /e %ModuleRootPath%\all_libs   %ModuleRootPath%\open_src\all_libs
xcopy /y /e %ModuleRootPath%\open_src\chksgfiles\chksgfiles.lib   %ModuleRootPath%\open_src\all_libs
xcopy /y /e %ModuleRootPath%\open_src\chksgfiles\ese.dll   %ModuleRootPath%\open_src\all_libs
xcopy /y /e %ModuleRootPath%\open_src\chksgfiles\chksgfiles.dll   %ModuleRootPath%\open_src\all_libs

IF exist %ModuleRootPath%\EXT_Pkg  rd /S /Q %ModuleRootPath%\EXT_Pkg

rem vs2017 has bug not support the alias of fuction pointer,so need to replace it.
git-bash.exe -c "sed -i 's/using signature = void(BOOST_PROCESS_V2_NAMESPACE::error_code);/typedef void(\*signature)(BOOST_PROCESS_V2_NAMESPACE::error_code);/g' ../../Windows_Compile/Module/open_src/boost/.libs/include/boost/process/v2/exit_code.hpp"

endlocal
