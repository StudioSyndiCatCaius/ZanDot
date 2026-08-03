@echo off
setlocal EnableExtensions
cd /d "%~dp0"

REM Default: Liberty Belle Christmas game project addons folder.
REM Override with: copy_to_project.bat "D:\path\to\project\addons\ZanDot"
set "DEST=%~dp0..\..\Projects\liberty-belle-christmas\addons\ZanDot"
if not "%~1"=="" set "DEST=%~1"

echo === Copy ZanDot into Godot project ===
echo Source: %~dp0
echo Dest:   %DEST%

if not exist "zandot.gdextension" (
    echo Missing zandot.gdextension in ZanDot folder.
    exit /b 1
)

if not exist "bin" (
    echo Missing bin\ folder. Build first with build.bat
    exit /b 1
)

dir /b "bin\*.dll" >nul 2>&1
if errorlevel 1 (
    dir /b "bin\*.so" >nul 2>&1
    if errorlevel 1 (
        dir /b "bin\*.dylib" >nul 2>&1
        if errorlevel 1 (
            dir /b "bin\*.framework" >nul 2>&1
            if errorlevel 1 (
                echo No built libraries found in bin\. Run build.bat first.
                exit /b 1
            )
        )
    )
)

if not exist "%DEST%" mkdir "%DEST%"
if not exist "%DEST%\bin" mkdir "%DEST%\bin"

copy /Y "zandot.gdextension" "%DEST%\zandot.gdextension" >nul
if errorlevel 1 (
    echo Failed to copy zandot.gdextension
    exit /b 1
)

REM Copy only runtime libraries (skip .lib/.exp/.pdb intermediates).
set COPIED=0
for %%F in (bin\*.dll bin\*.so bin\*.dylib) do (
    if exist "%%F" (
        copy /Y "%%F" "%DEST%\bin\" >nul
        set COPIED=1
    )
)
if exist "bin\*.framework" (
    xcopy /Y /E /I "bin\*.framework" "%DEST%\bin\" >nul
    set COPIED=1
)

if "%COPIED%"=="0" (
    echo Failed to copy runtime libraries from bin\
    exit /b 1
)

echo.
echo Installed ZanDot to:
echo   %DEST%\zandot.gdextension
echo   %DEST%\bin\
echo.
echo Restart or reload the Godot editor if it is already open.
echo Create resources via: Create New -^> Resource -^> ZResource
exit /b 0
