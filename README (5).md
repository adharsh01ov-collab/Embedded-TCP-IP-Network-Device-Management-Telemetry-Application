# Embedded TCP/IP Network Device Manager

[![Build](https://github.com/<your-username>/EmbeddedNetworkMonitor/actions/workflows/build.yml/badge.svg)](https://github.com/<your-username>/EmbeddedNetworkMonitor/actions)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

A C++ / Winsock2 project that simulates an embedded device (ECU) talking to a central device manager over TCP/IP.

It demonstrates TCP socket programming, device registration, continuous telemetry, heartbeat monitoring, newline-based message framing, acknowledgements, and basic network fault handling.

![Device and server running](docs/screenshots/device-and-server-running.png)

---

## Table of Contents

1. [Features](#features)
2. [Architecture](#architecture)
3. [Quick Start](#quick-start)
4. [Protocol](#protocol)
5. [Project Structure](#project-structure)
6. [How It Works](#how-it-works)
7. [Troubleshooting](#troubleshooting)
8. [Current Limitations](#current-limitations)
9. [Roadmap](#roadmap)
10. [What This Project Demonstrates](#what-this-project-demonstrates)
11. [License](#license)

---

## Features

**Device simulator**
- Connects to the server over TCP
- Registers with a unique ID (`DEV001`, type `ECU`)
- Sends telemetry (temperature, speed, voltage) every 3 seconds
- Sends a heartbeat after every 2 telemetry messages
- Detects transmission failures and shuts down cleanly

**Device manager (server)**
- Initializes Winsock, creates, binds and listens on a TCP socket
- Accepts a device connection
- Reassembles messages from the TCP byte stream using `\n` framing
- Parses and displays `REGISTER`, `TELEMETRY` and `HEARTBEAT` messages
- Replies `ACK` for every complete message
- Detects graceful disconnects and socket errors

---

## Architecture

```text
                    TCP/IP NETWORK
                          |
                  +-------v--------+
                  | Device Manager |
                  |     SERVER     |
                  | 127.0.0.1:5000 |
                  +-------+--------+
                          |
                          | TCP
                          |
                  +-------v--------+
                  | Embedded Device|
                  |   Simulator    |
                  | DEV001 / ECU   |
                  +----------------+
```

Communication flow:

```text
DEVICE                              SERVER
  |-------- TCP CONNECT -------------->|
  |-------- REGISTER|DEV001|ECU ------>|
  |<------------- ACK -----------------|
  |-------- TELEMETRY ---------------->|
  |<------------- ACK -----------------|
  |-------- TELEMETRY ---------------->|
  |<------------- ACK -----------------|
  |-------- HEARTBEAT ---------------->|
  |<------------- ACK -----------------|
  |             ... repeats ...        |
```

More detail in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## Quick Start

### Requirements

- Windows
- [MSYS2](https://www.msys2.org/) with the **UCRT64** environment
- `g++` (install with `pacman -S mingw-w64-ucrt-x86_64-gcc`)

Verify:

```bash
g++ --version
```

### Build

```bash
git clone https://github.com/<your-username>/EmbeddedNetworkMonitor.git
cd EmbeddedNetworkMonitor
make            # or: ./build.sh
```

Or compile each program manually:

```bash
g++ server/server.cpp -o server/server.exe -lws2_32
g++ device/device.cpp -o device/device.exe -lws2_32
```

> `-lws2_32` links the Windows Winsock library. It must come **after** the source file, and note it is a lowercase **L** (`-lws2_32`), not `lws2_32` without the dash.

### Run

Open two terminals.

**Terminal 1: server**

```bash
cd server
./server.exe
```

```text
======================================
 Embedded TCP/IP Network Device Manager
======================================
[OK] Winsock initialized.
[OK] TCP socket created.
[OK] Server bound to 127.0.0.1:5000
[OK] Server listening...
```

**Terminal 2: device** (start it *after* the server)

```bash
cd device
./device.exe
```

```text
======================================
 Embedded Device Simulator
======================================
[OK] Winsock initialized.
[OK] TCP socket created.
[CONNECTED] Connected to server.
[SENT] Device registration.
[SENT] Telemetry -> Temp=28.7 C, Speed=1250 RPM
[SENT] Telemetry -> Temp=28.9 C, Speed=1300 RPM
[SENT] Heartbeat.
```

Stop either program with `Ctrl + C`. When the device exits, the server prints `[DISCONNECTED] Device disconnected.`

---

## Protocol

A simple text protocol over TCP. Each message is one line, fields separated by `|`, terminated with `\n`.

| Message | Format | Example |
|---|---|---|
| Register | `REGISTER\|<id>\|<type>` | `REGISTER\|DEV001\|ECU` |
| Telemetry | `TELEMETRY\|<id>\|<temp>\|<speed>\|<voltage>` | `TELEMETRY\|DEV001\|28.7\|1250\|3.30` |
| Heartbeat | `HEARTBEAT\|<id>` | `HEARTBEAT\|DEV001` |
| Acknowledgement | `ACK` | `ACK` |

**Why framing matters:** TCP is a byte stream and does not preserve message boundaries. Two `send()` calls can arrive in one `recv()`, or one message can be split across two. The server therefore appends received bytes to a buffer and extracts a message each time it finds `\n`.

Full specification: [docs/PROTOCOL.md](docs/PROTOCOL.md)

---

## Project Structure

```text
EmbeddedNetworkMonitor/
├── README.md
├── LICENSE
├── Makefile
├── build.sh
├── build.bat
├── .gitignore
├── .github/workflows/build.yml   CI build on Windows
├── server/
│   └── server.cpp                Device manager
├── device/
│   └── device.cpp                Embedded device simulator
├── protocol/
│   └── README.md                 Message reference
├── logs/                         Reserved for runtime logs
└── docs/
    ├── ARCHITECTURE.md
    ├── PROTOCOL.md
    ├── TROUBLESHOOTING.md
    ├── ROADMAP.md
    └── screenshots/
```

---

## How It Works

**Server socket lifecycle**

```text
WSAStartup -> socket -> bind -> listen -> accept -> recv -> process -> send ACK -> recv ... -> closesocket -> WSACleanup
```

**Device socket lifecycle**

```text
WSAStartup -> socket -> connect -> send REGISTER -> send TELEMETRY / HEARTBEAT loop ... -> closesocket -> WSACleanup
```

**`sendAll()`**: a single `send()` is not guaranteed to transmit every byte. `sendAll()` loops until the whole message has been sent.

**Telemetry simulation** (starting values, updated every 3 s):

| Field | Start | Change per cycle |
|---|---|---|
| Temperature | 28.5 C | +0.2 |
| Speed | 1200 RPM | +50 |
| Voltage | 3.30 V | constant |

**Disconnect detection:** `recv()` returning `0` means the peer closed the connection gracefully; `SOCKET_ERROR` triggers a report using `WSAGetLastError()`.

---

## Troubleshooting

| Symptom | Cause / Fix |
|---|---|
| `cannot find lws2_32` | Missing the dash. Use `-lws2_32`. |
| `./device.exe: No such file or directory` | Not compiled yet, or wrong directory. Run `pwd`, `ls`, then rebuild. |
| `[ERROR] Connection failed` on device | Server is not running. Start the server first. |
| `[ERROR] Telemetry transmission failed` | Server was stopped while the device was running. |
| `Bind failed. Error: 10048` | Port 5000 already in use (an old server instance is still running). |
| `nano: command not found` | Install with `pacman -S nano` in MSYS2. |
| Pasting into nano misbehaves | Use `Ctrl + Shift + V` or right-click paste, not `Ctrl + V`. |

More in [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

---

## Current Limitations

- One device at a time (single client, blocking `accept`)
- Localhost only (`127.0.0.1`)
- The device does not yet read the server's `ACK` replies
- No heartbeat timeout, reconnection, authentication, encryption or persistence
- Telemetry values are simulated, not read from real hardware

---

## Roadmap

1. Multiple devices (threads or `select()`)
2. Device state table (ONLINE / OFFLINE, last heartbeat)
3. Heartbeat timeout detection
4. Automatic device reconnection
5. Command and control (`START`, `STOP`, `RESET`, `SET_SPEED`)
6. Wireshark analysis of the TCP handshake, data and teardown
7. Real embedded hardware over Ethernet

Details: [docs/ROADMAP.md](docs/ROADMAP.md)

---

## What This Project Demonstrates

- C++ socket programming with Winsock2
- TCP connection lifecycle and client-server design
- Application-layer protocol design and message framing
- Telemetry and heartbeat mechanisms
- Error handling and clean resource cleanup
- Embedded device simulation

---

## License

Released under the [MIT License](LICENSE).

Author: Adharsh V, B.E. Electronics and Communication Engineering, Saveetha Engineering College.
