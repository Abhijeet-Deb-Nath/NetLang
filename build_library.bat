@echo off
REM Build NetLang network as shared library (DLL)
REM This allows loading once and calling many times with no process overhead

if "%~1"=="" (
    echo Usage: build_library.bat ^<network.c^> [output_name]
    echo Example: build_library.bat generated\lenet5.c netlang_lenet5
    exit /b 1
)

set NETWORK=%~1
set OUTPUT=%~2
if "%OUTPUT%"=="" set OUTPUT=%~n1

if not exist bin mkdir bin

echo Building %NETWORK% as shared library...
echo Output: bin\%OUTPUT%.dll

gcc -O3 -march=haswell -mavx2 -mfma -DNDEBUG ^
    -DNETLANG_NO_MAIN ^
    -shared ^
    -I. ^
    %NETWORK% ^
    src\codegen\runtime.c ^
    src\codegen\kernels.c ^
    -o bin\%OUTPUT%.dll ^
    -Wl,--out-implib,bin\%OUTPUT%.lib ^
    -lm

if errorlevel 1 (
    echo BUILD FAILED!
    exit /b 1
)

echo.
echo SUCCESS: bin\%OUTPUT%.dll
echo.
echo This DLL exports:
echo   - network_init()
echo   - network_infer()
echo   - network_cleanup()
echo.
echo Use with Python (ctypes) or C/C++ applications
