@echo off
REM Build script for VS Code using MSYS2 tools

echo ============================================
echo NetLang Compiler Build Script (MSYS2)
echo ============================================

if exist build rmdir /s /q build
mkdir build

echo [1/4] Running Bison...
C:\msys64\usr\bin\bison.exe -d -v -o build\net_lang.tab.c src\parser\net_lang.y
if errorlevel 1 goto error

echo [2/4] Running Flex...
C:\msys64\usr\bin\flex.exe -o build\lex.yy.c src\lexer\net_lang.l
if errorlevel 1 goto error

echo [3/4] Compiling...
gcc -Wall -g -c -o build\lex.yy.o build\lex.yy.c -Ibuild -Isrc\ast
if errorlevel 1 goto error

gcc -Wall -g -c -o build\net_lang.tab.o build\net_lang.tab.c -Ibuild -Isrc\ast
if errorlevel 1 goto error

gcc -Wall -g -c -o build\ast.o src\ast\ast.c -Isrc\ast
if errorlevel 1 goto error

echo [4/4] Linking...
gcc -Wall -g -o build\netlang.exe build\lex.yy.o build\net_lang.tab.o build\ast.o
if errorlevel 1 goto error

echo.
echo ============================================
echo BUILD SUCCESSFUL!
echo Output: build\netlang.exe
echo ============================================
echo.
echo Run with: build\netlang.exe examples\demo.nlang
goto end

:error
echo ERROR: Build failed!
exit /b 1

:end