# Program Output

Output of the **Embedded TCP/IP Network Device Manager**: a C++ / Winsock2 device simulator (TCP client) talking to a device manager (TCP server) on `127.0.0.1:5000`.

In every screenshot the **left terminal** is the device (`~/EmbeddedNetworkMonitor/device`) and the **right terminal** is the server (`~/EmbeddedNetworkMonitor/server`), both running in MSYS2 UCRT64.

---

## 1. Source code in nano

![Source code in nano](docs/screenshots/01-source-code-nano.png)

Both programs were written with the `nano` editor. `device.cpp` (left, 77 lines) and `server.cpp` (right, 72 lines) are shown here in their first, simpler version.

What the code does at this stage:

| Step | Device (`device.cpp`) | Server (`server.cpp`) |
|---|---|---|
| 1 | `WSAStartup()` initializes Winsock | `WSAStartup()` initializes Winsock |
| 2 | `socket()` creates a TCP socket | `socket()` creates a TCP socket |
| 3 | Fills `sockaddr_in` with `127.0.0.1:5000` | Fills `sockaddr_in` with `127.0.0.1:5000` |
| 4 | `connect()` to the server | `bind()`, `listen()`, then `accept()` |

Each step checks for errors (`INVALID_SOCKET`, `SOCKET_ERROR`), cleans up with `WSACleanup()`, and returns `1` on failure.

---

## 2. Compiling and starting the programs

![Build and start](docs/screenshots/02-build-and-start.png)

This screenshot shows the development workflow, including the mistakes made along the way.

**Compile error on the server (right terminal)**

```text
$ g++ server.cpp -o server.exe lws2_32
ld.exe: cannot find lws2_32: No such file or directory
```

The linker flag was typed without the dash. The Winsock library must be linked with `-lws2_32`. The next attempt used the correct command and produced `server.exe`:

```text
$ g++ server.cpp -o server.exe -lws2_32
$ ls
server.cpp  server.cpp.save  server.exe
```

(`server.cpp.save` is a backup nano created when an edit was interrupted with `Ctrl + C`.)

**Device side (left terminal)**

- `nao: command not found` was a typo for `nano`.
- `./device.exe: No such file or directory` appeared after `rm device.exe` removed the executable. Deleting the `.exe` does not delete the source; it has to be recompiled.
- `cd ~/EmbeddedDeviceMonitor/device` failed because the real folder is `EmbeddedNetworkMonitor`.
- After `g++ device.cpp -o device.exe -lws2_32` the executable was rebuilt.

**First run of the new device build**

```text
[OK] Winsock initialized.
[OK] TCP socket created.
[CONNECTED] Connected to server.
[SENT] Device registration.
[ERROR] Telemetry transmission failed.
Device shutdown complete.
```

The connection succeeded and registration was sent, but the next send failed. This happened because the server on the other side was still the **older build**, which printed `Client connected successfully!` and then exited, closing the connection. The device correctly detected the failed `send()` and shut down cleanly. This is the network fault handling working as intended. The new server (bottom right, `Embedded TCP/IP Network Device Manager`) was then started and is waiting at `Server listening...`.

---

## 3. Registration and telemetry received

![Registration and telemetry](docs/screenshots/03-registration-and-telemetry.png)

With the new server listening, the device was started again and the full exchange worked.

**Device (left)**

```text
[CONNECTED] Connected to server.
[SENT] Device registration.
[SENT] Telemetry -> Temp=28.7 C, Speed=1250 RPM
[SENT] Telemetry -> Temp=28.9 C, Speed=1300 RPM
[SENT] Heartbeat.
```

**Server (right)**

```text
[CONNECTED] Device connected.

[REGISTER]
Device ID   : DEV001
Device Type : ECU
Status      : ONLINE

[TELEMETRY]
Device ID   : DEV001
Temperature : 28.700000 C
Speed       : 1250.000000 RPM
Voltage     : 3.300000 V

[HEARTBEAT]
Device ID : DEV001
Status    : ALIVE
```

How to read this:

1. **TCP connection:** the server's `accept()` returns and prints `[CONNECTED]`.
2. **Registration:** the device sends `REGISTER|DEV001|ECU\n`. The server parses the three fields and marks the device `ONLINE`.
3. **Telemetry:** every 3 seconds the device sends `TELEMETRY|DEV001|<temp>|<speed>|<voltage>\n`. Temperature rises 0.2 C and speed rises 50 RPM each cycle, starting from 28.5 C and 1200 RPM.
4. **Heartbeat:** after every second telemetry message the device sends `HEARTBEAT|DEV001\n`, and the server reports the device `ALIVE`.
5. **Message framing:** the server splits the incoming byte stream on `\n`, so each message is processed separately even though TCP does not preserve boundaries.
6. **Number format:** the device prints `28.7` locally, but the server shows `28.700000`. The device builds the message with `std::to_string()`, which sends six decimal places on the wire.

---

## 4. Continuous operation and shutdown

![Continuous run and shutdown](docs/screenshots/04-continuous-run-and-shutdown.png)

A longer run showing the stable repeating pattern.

**Device (left)** sent telemetry from `Temp=28.7 C, Speed=1250 RPM` up to `Temp=32.5 C, Speed=2200 RPM`, with a `[SENT] Heartbeat.` after every two telemetry lines:

```text
Telemetry -> Temp=31.7 C, Speed=2000 RPM
Heartbeat.
Telemetry -> Temp=31.9 C, Speed=2050 RPM
Telemetry -> Temp=32.1 C, Speed=2100 RPM
Heartbeat.
Telemetry -> Temp=32.3 C, Speed=2150 RPM
Telemetry -> Temp=32.5 C, Speed=2200 RPM
Heartbeat.
```

**Server (right)** shows the matching received values (`31.300000 C / 1900 RPM` through `32.500000 C / 2200 RPM`), each `[TELEMETRY]` block followed by `[HEARTBEAT]` on the same two-to-one rhythm. Voltage stays at `3.300000 V` throughout.

**Why the device ends with an error**

```text
[ERROR] Telemetry transmission failed.
Device shutdown complete.
```

The server was stopped (its terminal is back at the `$` prompt). On the next `send()` the device detected that the connection was gone, reported it, closed its socket, called `WSACleanup()` and exited without crashing.

---

## Summary

| Behaviour | Seen in |
|---|---|
| Winsock initialization and TCP socket creation | Screenshots 2 and 3 |
| Server bind, listen and accept | Screenshots 2 and 3 |
| Device registration (`REGISTER`) | Screenshot 3 |
| Periodic telemetry every 3 s | Screenshots 3 and 4 |
| Heartbeat after every 2 telemetry messages | Screenshots 3 and 4 |
| Newline-based message framing | Screenshots 3 and 4 |
| Fault detection when the peer disappears | Screenshots 2 and 4 |
| Clean shutdown (`closesocket`, `WSACleanup`) | Screenshots 2 and 4 |

Not visible in these screenshots: the server's `ACK` replies. The server sends them, but the device does not read or print them yet. See [docs/ROADMAP.md](docs/ROADMAP.md).
