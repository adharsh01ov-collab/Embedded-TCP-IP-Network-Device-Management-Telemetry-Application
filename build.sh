#!/usr/bin/env bash
# Build both programs (MSYS2 UCRT64 / MinGW g++)
set -e
cd "$(dirname "$0")"

echo "Building server..."
g++ -std=c++11 -Wall -Wextra server/server.cpp -o server/server.exe -lws2_32

echo "Building device..."
g++ -std=c++11 -Wall -Wextra device/device.cpp -o device/device.exe -lws2_32

echo "Done."
echo "  Terminal 1: ./server/server.exe"
echo "  Terminal 2: ./device/device.exe"
