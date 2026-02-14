@echo off
setlocal

set MAKE_PATH=E:\Program Files\STM32CubeIDE_2.0.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin\make.exe
set PROJECT_PATH=e:\STM32\OV\Release

cd /d "%PROJECT_PATH%"

echo ========================================
echo Single-threaded build for clear errors
echo ========================================
"%MAKE_PATH%" all 2>&1

if %ERRORLEVEL% EQU 0 (
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo ========================================
) else (
    echo ========================================
    echo BUILD FAILED! Error code: %ERRORLEVEL%
    echo ========================================
)
