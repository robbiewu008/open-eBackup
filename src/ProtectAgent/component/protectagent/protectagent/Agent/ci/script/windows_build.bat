rem windows������
cd %WORKSPACE%\Agent\build\ms
call download_opensrc.bat
call agent_pack.bat
exit %ERRORLEVEL%