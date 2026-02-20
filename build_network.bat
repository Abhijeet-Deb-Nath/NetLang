@echo off
REM Simple build script for generated network executables
REM Usage: build_network.bat generated\lenet5.c output_name

if "%~1"=="" (
    echo Usage: build_network.bat ^<network.c^> [output_name]
    echo Example: build_network.bat generated\lenet5_optimized.c test_lenet5_opt
    exit /b 1
)

set NETWORK=%~1
set OUTPUT=%~2
if "%OUTPUT%"=="" set OUTPUT=%~n1

if not exist bin mkdir bin

echo Building %NETWORK% -^> bin\%OUTPUT%.exe

gcc -O3 -march=haswell -mavx2 -mfma -DNDEBUG -DNETLANG_NO_MAIN -I. ^
    %NETWORK% ^
    src\codegen\runtime.c ^
    src\codegen\kernels.c ^
    tools\test_network.c ^
    -o bin\%OUTPUT%.exe -lm

if errorlevel 1 (
    echo BUILD FAILED!
    exit /b 1
)

echo SUCCESS: bin\%OUTPUT%.exe
