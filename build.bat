@echo off
setlocal EnableExtensions
cd /d "%~dp0"

echo === ZanDot build ===

if not exist "godot-cpp\SConstruct" (
    echo godot-cpp is missing. Cloning godot-cpp 4.5 branch...
    git clone --depth 1 -b 4.5 https://github.com/godotengine/godot-cpp.git godot-cpp
    if errorlevel 1 (
        echo Failed to clone godot-cpp.
        exit /b 1
    )
    pushd godot-cpp
    git submodule update --init --recursive
    popd
)

where cl >nul 2>&1
if errorlevel 1 (
    echo MSVC cl.exe not on PATH. Loading Visual Studio environment...
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles(x86)%\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        echo Could not find vcvars64.bat. Install Visual Studio C++ tools and retry.
        exit /b 1
    )
)

where scons >nul 2>&1
if errorlevel 1 (
    echo scons not found on PATH. Install with: pip install scons
    exit /b 1
)

set "TARGET=template_debug"
if /I "%~1"=="release" set "TARGET=template_release"
if /I "%~1"=="template_release" set "TARGET=template_release"
if /I "%~1"=="debug" set "TARGET=template_debug"
if /I "%~1"=="template_debug" set "TARGET=template_debug"

echo Building platform=windows target=%TARGET%
scons platform=windows target=%TARGET%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo.
echo Build succeeded. Output is in ZanDot\bin\
echo Run copy_to_project.bat to install into the Godot game project.
exit /b 0
