# Build with MSYS2 UCRT64 / MinGW g++ on Windows
CXX      := g++
CXXFLAGS := -std=c++11 -Wall -Wextra
LDLIBS   := -lws2_32

.PHONY: all server device clean run-server run-device

all: server device

server: server/server.exe
device: device/device.exe

server/server.exe: server/server.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

device/device.exe: device/device.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

run-server: server
	./server/server.exe

run-device: device
	./device/device.exe

clean:
	rm -f server/server.exe device/device.exe
