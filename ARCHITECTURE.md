# Architecture

## Overview

Two standalone programs communicate over a TCP connection on the local machine.

```text
+-------------------+        TCP 127.0.0.1:5000        +---------------------+
| Device Simulator  | -------------------------------> | Device Manager      |
| device.exe        | <------------------------------- | server.exe          |
| (TCP client)      |              ACK                 | (TCP server)        |
+-------------------+                                  +---------------------+
```

## Components

### Device Simulator (`device/device.cpp`)

| Part | Role |
|---|---|
| `main()` | Initializes Winsock, connects, registers, then loops forever sending telemetry and heartbeats |
| `sendAll()` | Sends the complete message even if `send()` transmits only part of it |
| Telemetry model | Temperature rises 0.2 C and speed rises 50 RPM per cycle; voltage fixed at 3.30 V |

### Device Manager (`server/server.cpp`)

| Part | Role |
|---|---|
| `main()` | Winsock init, socket, bind, listen, accept, receive loop, cleanup |
| Receive loop | Appends bytes to `receiveBuffer`, extracts one message per `\n` |
| `processMessage()` | Splits on `|`, dispatches on message type, prints formatted output |
| `sendAll()` | Sends `ACK\n` after each processed message |

## Server flow

```text
WSAStartup
   |
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
   |
bind(127.0.0.1:5000)
   |
listen(backlog 5)
   |
accept()  -- blocks until a device connects
   |
loop:
   recv() -> append to buffer
   while buffer contains '\n':
       extract line -> processMessage() -> send "ACK\n"
   recv()==0     -> device disconnected, exit loop
   recv()==ERROR -> report WSAGetLastError(), exit loop
   |
closesocket(client), closesocket(server), WSACleanup
```

## Device flow

```text
WSAStartup -> socket -> connect -> send REGISTER
   |
loop every 3 s:
   update simulated values
   send TELEMETRY
   every 2nd cycle: send HEARTBEAT
   on send failure: report and exit
   |
closesocket -> WSACleanup
```

## Message framing

```text
bytes received:   "TELEMETRY|DEV001|28.7|1250|3.30\nHEART"
                                                    ^ incomplete
buffer after extracting first line:   "HEART"
next recv() delivers:                 "BEAT|DEV001\n"
buffer now:                           "HEARTBEAT|DEV001\n"  -> complete message
```

## Design decisions

- **Text protocol:** easy to read in a terminal and in Wireshark.
- **Newline framing:** simplest reliable way to recover message boundaries from a TCP stream.
- **Single-threaded server:** keeps the first version easy to follow; multi-device support is the first roadmap item.
- **Localhost binding:** safe default for development; change the bind address to accept LAN devices.

## Planned architecture (multi-device)

```text
DEV001 ----\
DEV002 -----+--->  Device Manager  --->  state table  --->  monitoring UI / logs
DEV003 ----/
```
