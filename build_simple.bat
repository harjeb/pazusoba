@echo off
REM Simple MSBuild script for pazusoba

REM Check if msbuild is available
where msbuild >nul 2>nul
if errorlevel 1 (
    echo MSBuild not found in PATH
    echo Please run this from a Visual Studio Developer Command Prompt
    echo Or run build.bat which sets up the environment automatically
    pause
    exit /b 1
)

REM Create directories
if not exist "bin" mkdir "bin"
if not exist "bin\Debug" mkdir "bin\Debug"
if not exist "bin\Release" mkdir "bin\Release"

echo Building pazusoba solution...
msbuild pazusoba.sln /p:Configuration=Release /p:Platform=x64 /v:minimal

if errorlevel 1 (
    echo Build failed
    pause
    exit /b 1
) else (
    echo Build successful!
    echo Executables are in bin\Release\
)

pause