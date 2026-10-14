@echo off
setlocal
cd /d "%~dp0"
set "DXFFIX_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%DXFFIX_VSWHERE%" exit /b 1
for /f "usebackq tokens=*" %%I in (`"%DXFFIX_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "DXFFIX_VS=%%I"
if not defined DXFFIX_VS exit /b 1
call "%DXFFIX_VS%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
MSBuild DXF_fix.slnx /p:Configuration=Release /p:Platform=x64 /verbosity:minimal /nologo
exit /b %errorlevel%
