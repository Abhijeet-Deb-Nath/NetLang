@echo off
REM Build script for generated network benchmark executables
REM Usage: build_benchmark.bat generated\lenet5.c output_name

if "%~1"=="" (
    echo Usage: build_benchmark.bat ^<network.c^> [output_name]
    echo Example: build_benchmark.bat generated\lenet5.c lenet5_bench
    exit /b 1
)

set NETWORK=%~1
set OUTPUT=%~2
if "%OUTPUT%"=="" set OUTPUT=%~n1_bench

if not exist bin mkdir bin
if "%GEN_CFLAGS%"=="" set GEN_CFLAGS=-O3 -march=haswell -mavx2 -mfma -DNDEBUG -DNETLANG_NO_MAIN -I.

echo Building benchmark %NETWORK% -^> bin\%OUTPUT%.exe

gcc %GEN_CFLAGS% ^
    %NETWORK% ^
    src\codegen\runtime.c ^
    src\codegen\kernels.c ^
    tools\benchmark.c ^
    -o bin\%OUTPUT%.exe -lm

if errorlevel 1 (
    echo BUILD FAILED!
    exit /b 1
)

echo SUCCESS: bin\%OUTPUT%.exe
