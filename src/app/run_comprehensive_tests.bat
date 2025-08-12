@echo off
echo ========================================
echo BCD Event List Generation Test Suite
echo ========================================
echo.

echo Building and running comprehensive validation tests...
echo.

echo [1/3] Running BCD Error Handling Tests...
echo ----------------------------------------
make test-bcd-error-handling
if %ERRORLEVEL% neq 0 (
    echo ERROR: BCD Error Handling tests failed!
    exit /b 1
)
echo.

echo [2/3] Running BCD Comprehensive Validation Tests...
echo --------------------------------------------------
make test-bcd-comprehensive
if %ERRORLEVEL% neq 0 (
    echo ERROR: BCD Comprehensive Validation tests failed!
    exit /b 1
)
echo.

echo [3/3] Running BCD JSON Pipeline Tests...
echo ----------------------------------------
make test-bcd-json-pipeline
if %ERRORLEVEL% neq 0 (
    echo ERROR: BCD JSON Pipeline tests failed!
    exit /b 1
)
echo.

echo ========================================
echo All comprehensive tests completed successfully!
echo ========================================
echo.
echo Test Coverage Summary:
echo - Test data sets: Simple rectangle, rectangle with obstacle, complex environment
echo - Geometric validation: Edge angle computation, event classification
echo - Pipeline validation: JSON input to sorted event list output
echo - Error handling: Input validation, memory management, constraint checking
echo - Requirements validated: 4.2, 4.3, 4.5
echo.