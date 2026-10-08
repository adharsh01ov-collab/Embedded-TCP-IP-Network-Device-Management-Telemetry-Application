@echo off
REM Build both programs with MinGW g++ from a Windows cmd prompt
cd /d "%~dp0"

echo Building server...
g++ -std=c++11 -Wall -Wextra server\server.cpp -o server\server.exe -lws2_32 || exit /b 1

echo Building device...
g++ -std=c++11 -Wall -Wextra device\device.cpp -o device\device.exe -lws2_32 || exit /b 1

echo Done.
echo   Terminal 1: server\server.exe
echo   Terminal 2: device\device.exe
