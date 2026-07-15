@echo off
setlocal

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 exit /b 1

cl /nologo /std:c++17 /EHsc /MT /LD /DWIN32 /D_WINDOWS ^
 /Fe:"%~dp0RCPilotT.asi" ^
 "%~dp0RCPilotT.cpp" ^
 user32.lib kernel32.lib

endlocal
