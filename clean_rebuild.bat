@echo off
echo Cleaning and rebuilding Pazusoba project...
echo ==========================================

REM Clean previous builds
if exist "bin" rmdir /s /q "bin"
if exist "obj" rmdir /s /q "obj"
if exist "*.log" del "*.log"

REM Set Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
    if errorlevel 1 (
        echo Error: Visual Studio environment not found
        echo Please make sure Visual Studio 2019 or 2022 is installed
        pause
        exit /b 1
    )
)

echo Visual Studio environment loaded successfully
echo.

REM Create output directories
if not exist "bin\Debug" mkdir "bin\Debug"
if not exist "bin\Release" mkdir "bin\Release"
if not exist "obj" mkdir "obj"

echo Cleaning solution...
msbuild pazusoba.sln /t:Clean /p:Configuration=Debug /p:Platform=x64
msbuild pazusoba.sln /t:Clean /p:Configuration=Release /p:Platform=x64

echo.
echo Rebuilding Debug configuration...
msbuild pazusoba.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /m
if errorlevel 1 (
    echo Debug rebuild failed
    pause
    exit /b 1
)

echo.
echo Rebuilding Release configuration...
msbuild pazusoba.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m
if errorlevel 1 (
    echo Release rebuild failed
    pause
    exit /b 1
)

echo.
echo Clean rebuild completed successfully!
echo Debug executables: bin\Debug\
echo Release executables: bin\Release\
pause