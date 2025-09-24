@echo off
REM Manual version bump script for Windows
REM Usage: bump-version.bat [commit-message]

if "%~1"=="" (
    echo Usage: bump-version.bat "commit message"
    echo Example: bump-version.bat "fix: resolve servo positioning bug"
    exit /b 1
)

echo Bumping version for commit: %~1

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    python3 --version >nul 2>&1
    if errorlevel 1 (
        echo Error: Python not found. Please install Python.
        exit /b 1
    ) else (
        set PYTHON_CMD=python3
    )
) else (
    set PYTHON_CMD=python
)

REM Run version bump script
if exist "scripts\bump_version.py" (
    %PYTHON_CMD% scripts\bump_version.py %~1
    if errorlevel 1 (
        echo Error: Version bump failed
        exit /b 1
    )
    echo Version bump completed successfully
) else (
    echo Error: Version bump script not found at scripts\bump_version.py
    exit /b 1
)