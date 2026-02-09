@echo off
REM Test suite for NetLang Compiler - Phase 3
REM Runs all semantic analysis tests

echo ============================================
echo NetLang Compiler - Phase 3 Test Suite
echo ============================================
echo.

echo [1/3] Testing valid network...
build\netlang.exe examples\test_valid.nlang > test_results\valid.txt 2>&1
findstr /C:"passed" test_results\valid.txt >nul
if %errorlevel%==0 (
    echo   [PASS] Valid network accepted
) else (
    echo   [FAIL] Valid network rejected
)

echo.
echo [2/3] Testing error detection...
build\netlang.exe examples\test_errors.nlang > test_results\errors.txt 2>&1
findstr /C:"failed" test_results\errors.txt >nul
if %errorlevel%==0 (
    echo   [PASS] Errors correctly detected
) else (
    echo   [FAIL] Errors not detected
)

echo.
echo [3/3] Testing demo networks...
build\netlang.exe examples\demo.nlang > test_results\demo.txt 2>&1
findstr /C:"passed" test_results\demo.txt >nul
if %errorlevel%==0 (
    echo   [PASS] Demo networks validated
) else (
    echo   [FAIL] Demo validation failed
)

echo.
echo ============================================
echo Test Results Summary:
echo ============================================
echo Full logs in test_results\ folder
echo.
