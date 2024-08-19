@echo off

REM Compile the source files
g++ -o main poly1305-donna.cpp rfc7539.cpp chacha20poly1305.cpp chacha_merged.cpp main.cpp  

REM Check if compilation was successful
if %errorlevel% equ 0 (
    echo Compilation successful.
) else (
    echo Compilation failed.
)