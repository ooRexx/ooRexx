@REM Use taskkill to make sure that rxapi.exe
@taskkill /F /IM rxapi.exe 2>NUL 1>NUL
@REM force errorlevel to 0 to keep nmake from killing the build if this is not active.
@EXIT /B 0
