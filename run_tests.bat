@echo off
echo Running DarkPdf Tests...

:: Navigate to build directory
if not exist build (
    echo Error: Build directory not found. Run run.bat first.
    pause
    exit /b 1
)

cd build

echo.
echo Method 2: Running test executable directly...
if exist tests\Debug\darkpdf_tests.exe (
    echo Running Debug tests...
    tests\Debug\darkpdf_tests.exe
) else if exist tests\Release\darkpdf_tests.exe (
    echo Running Release tests...
    tests\Release\darkpdf_tests.exe
) else (
    echo No test executable found!
)

echo.
echo Test execution completed.
pause
