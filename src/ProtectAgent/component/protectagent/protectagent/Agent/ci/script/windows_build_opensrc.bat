rem windows开源编包入口
cd %WORKSPACE%\Agent\build\ms
call download_opensrc.bat copy
call agent_pack.bat
exit %ERRORLEVEL%