@echo off
echo =======================================
echo   Teacher Free Slot Finder - C++
echo   Building project...
echo =======================================
echo.

echo Step 1: Compiling SQLite (C library)...
gcc -O2 -c sqlite\sqlite3.c -o sqlite\sqlite3.o
if %ERRORLEVEL% NEQ 0 ( echo [FAILED] SQLite compile. & pause & exit /b 1 )

echo Step 2: Compiling main C++ server...
g++ -std=c++14 -O2 main.cpp sqlite\sqlite3.o -o freeslot.exe -lws2_32 -I.
if %ERRORLEVEL% NEQ 0 ( echo [FAILED] C++ compile. & pause & exit /b 1 )

echo.
echo =======================================
echo   BUILD SUCCESS!
echo   Run : freeslot.exe
echo   URL : http://localhost:8080
echo =======================================
