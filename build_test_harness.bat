@echo off
REM Build NetLang Test Harness
REM Compiles the inference test system

echo Building NetLang Test Harness...
echo.

gcc -O3 -march=haswell -mavx2 -mfma -I. ^
    generated/generated_mnist.c ^
    src/codegen/runtime.c ^
    src/codegen/kernels.c ^
    tools/test_network.c ^
    -o bin/test_network.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build successful!
    echo ========================================
    echo.
    echo Executable: bin\test_network.exe
    echo.
    echo Quick test:
    echo   1. python tools\download_mnist_samples.py
    echo   2. python tools\preprocess.py test_data\images\ test_data\preprocessed\
    echo   3. bin\test_network.exe test_data\preprocessed\mnist_000_label_7.bin
    echo.
) else (
    echo.
    echo ========================================
    echo Build failed!
    echo ========================================
    echo.
    echo Make sure:
    echo   - GCC is installed and in PATH
    echo   - You have generated code in generated/
    echo   - All source files exist
    echo.
    exit /b 1
)
