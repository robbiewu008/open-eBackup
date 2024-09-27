rem windows±à°üÈë¿Ú
cd %WORKSPACE%\Agent\build\ms
call download_opensrc.bat
call pack_plugin_sdk.bat
exit %ERRORLEVEL%