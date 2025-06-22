@echo off
echo Building DarkPdf...

:: Clean build
if exist build rmdir /s /q build
mkdir build
cd build

:: Configure and build (works for both Debug/Release)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\Users\aryamanjain\vcpkg\scripts\buildsystems\vcpkg.cmake -A x64
cmake --build .

:: Find and show executable location
if exist Debug\darkpdf.exe (
    echo Success! Executable: build\Debug\darkpdf.exe
    echo Usage: build\Debug\darkpdf.exe input.pdf output.pdf
) else if exist Release\darkpdf.exe (
    echo Success! Executable: build\Release\darkpdf.exe  
    echo Usage: build\Release\darkpdf.exe input.pdf output.pdf
) else (
    echo Build failed!
)

pause