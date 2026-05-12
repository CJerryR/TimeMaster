@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul 2>&1

rem ============================================================
rem TimeMaster — Windows build script
rem
rem Usage:
rem   build_windows.bat             # auto-detect Qt, then build
rem   build_windows.bat C:\Qt\6.7.0\msvc2022_64
rem                                 # use explicit Qt prefix
rem
rem Requirements:
rem   - Qt 6.5+ for MSVC (download from https://www.qt.io/download
rem     or run "aqt install-qt windows desktop 6.7.0 win64_msvc2022_64")
rem   - Visual Studio 2019 or 2022 with "Desktop development with C++"
rem   - CMake 3.16+ (bundled with Visual Studio installer or qt.io)
rem
rem Output:
rem   build\Release\TimeMaster.exe and all required DLLs
rem ============================================================

echo.
echo ========================================
echo   TimeMaster Build Script
echo ========================================
echo.

rem ---- 1. Locate Qt ----
set "QT_PREFIX="
if not "%~1"=="" (
    set "QT_PREFIX=%~1"
    echo [INFO] Using Qt prefix from command line: !QT_PREFIX!
    goto :qt_found
)

if defined Qt6_DIR (
    set "QT_PREFIX=%Qt6_DIR%\..\..\.."
    echo [INFO] Using Qt6_DIR env var
    goto :qt_found
)

rem Common Qt install locations
for %%V in (6.11.1 6.11.0 6.10.3 6.10.2 6.10.1 6.10.0 6.9.3 6.9.2 6.9.1 6.9.0 6.8.3 6.8.2 6.8.1 6.8.0 6.7.3 6.7.2 6.7.1 6.7.0 6.6.3 6.6.2 6.6.1 6.6.0 6.5.3 6.5.2 6.5.1 6.5.0) do (
    for %%C in (msvc2022_64 msvc2019_64) do (
        if exist "C:\Qt\%%V\%%C\bin\qmake.exe" (
            set "QT_PREFIX=C:\Qt\%%V\%%C"
            echo [INFO] Auto-detected Qt at: !QT_PREFIX!
            goto :qt_found
        )
    )
)

echo.
echo [ERROR] Qt 6 for MSVC not found.
echo.
echo Please install Qt 6 (with MSVC component) from https://www.qt.io/download
echo Or pass the Qt prefix as the first argument, e.g.:
echo     build_windows.bat C:\Qt\6.7.0\msvc2022_64
echo.
pause
exit /b 1

:qt_found
if not exist "%QT_PREFIX%\bin\qmake.exe" (
    echo [ERROR] Invalid Qt prefix: %QT_PREFIX%
    echo         bin\qmake.exe was not found.
    pause
    exit /b 1
)

echo [INFO] Qt prefix    : %QT_PREFIX%

rem ---- 2. Locate MSVC ----
set "VCVARS="
for %%E in (Enterprise Professional Community BuildTools) do (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat" (
        set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat"
        goto :vc_found
    )
)
for %%E in (Enterprise Professional Community BuildTools) do (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat" (
        set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat"
        goto :vc_found
    )
)
for %%E in (Enterprise Professional Community BuildTools) do (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvars64.bat" (
        set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvars64.bat"
        goto :vc_found
    )
)

echo.
echo [WARN] Visual Studio (vcvars64.bat) was not auto-detected.
echo        If the build fails, open "x64 Native Tools Command Prompt for VS"
echo        and run this script from there.
echo.
goto :vc_skip

:vc_found
echo [INFO] MSVC vcvars  : %VCVARS%
echo [INFO] Running vcvars64 ...
call "%VCVARS%" >nul
if errorlevel 1 (
    echo [ERROR] vcvars64.bat returned an error.
    pause
    exit /b 1
)

:vc_skip

rem ---- 3. Configure ----
set "BUILD_DIR=%~dp0build"

echo.
echo [INFO] Build dir    : %BUILD_DIR%
echo.
echo ========================================
echo   [1/3] CMake configure
echo ========================================
echo.

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

rem 清理旧缓存（避免 cmake 源目录冲突）
if exist "%BUILD_DIR%\CMakeCache.txt" del /q "%BUILD_DIR%\CMakeCache.txt"

cmake -S "%~dp0." -B "%BUILD_DIR%" ^
    -DCMAKE_PREFIX_PATH="%QT_PREFIX%" ^
    -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo.
    echo [ERROR] CMake configure failed.
    pause
    exit /b 1
)
echo   [OK] Configure done.

rem ---- 4. Build ----
echo.
echo ========================================
echo   [2/3] Build (Release)
echo ========================================
echo.

rem 删除旧的 exe，避免被占用导致 LNK1104
if exist "%BUILD_DIR%\Release\TimeMaster.exe" del /q "%BUILD_DIR%\Release\TimeMaster.exe"

cmake --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    pause
    exit /b 1
)
echo   [OK] Build done.

rem ---- 5. Deploy Qt DLLs ----
echo.
echo ========================================
echo   [3/3] windeployqt
echo ========================================
echo.

set "EXE_DIR=%BUILD_DIR%\Release"
if not exist "%EXE_DIR%\TimeMaster.exe" (
    rem Multi-config might place it under different layouts
    if exist "%BUILD_DIR%\TimeMaster.exe" set "EXE_DIR=%BUILD_DIR%"
)

if not exist "%EXE_DIR%\TimeMaster.exe" (
    echo [ERROR] TimeMaster.exe not found after build.
    echo         Looked in: %EXE_DIR%
    exit /b 1
)

"%QT_PREFIX%\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler "%EXE_DIR%\TimeMaster.exe"
if errorlevel 1 (
    echo [WARN] windeployqt returned an error. The exe may still run if Qt is on PATH.
    pause
    exit /b 1
)
echo   [OK] windeployqt done.

echo.
echo ========================================
echo   BUILD SUCCEEDED
echo ========================================
echo.
echo Executable: %EXE_DIR%\TimeMaster.exe
echo.
echo You can zip up the whole "%EXE_DIR%" folder and share it.
echo Double-click TimeMaster.exe to launch.
echo.

pause
endlocal
exit /b 0
\r
