# Troubleshooting

## Build problems

### `cannot find lws2_32: No such file or directory`

The `-l` flag is missing its dash. Wrong:

```bash
g++ server.cpp -o server.exe lws2_32
```

Correct:

```bash
g++ server.cpp -o server.exe -lws2_32
```

### `g++: command not found`

Open the **MSYS2 UCRT64** terminal (not plain MSYS or Git Bash), then:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

### `nano: command not found`

```bash
pacman -S nano
```

## Run problems

### `./device.exe: No such file or directory`

The executable does not exist in the current directory. Check where you are and what is there:

```bash
pwd
ls
```

If only the `.cpp` file is listed, compile it:

```bash
g++ device.cpp -o device.exe -lws2_32
```

Remember: editing `device.cpp` does not change `device.exe`. Recompile after every edit.

### `cd: No such file or directory`

Check the exact folder name. The project folder is `EmbeddedNetworkMonitor`.

```bash
cd ~/EmbeddedNetworkMonitor/device
```

### Device prints `[ERROR] Connection failed`

The server is not running or is on a different port. Start `server.exe` first, then `device.exe`.

### Device prints `[ERROR] Telemetry transmission failed`

The server closed the connection (it was stopped or crashed). Restart the server, then the device.

### Server prints `Bind failed. Error: 10048`

Port 5000 is already in use, usually by a previous server instance that is still running. Close it, or change `SERVER_PORT` in both `server.cpp` and `device.cpp`.

### Server exits after the device disconnects

By design in v1.0: the server accepts one device and shuts down when it disconnects. Restart it to accept another. Multi-device support is on the roadmap.

## Editor tips (nano in MSYS2)

| Action | Keys |
|---|---|
| Save | `Ctrl + O`, then `Enter` |
| Exit | `Ctrl + X` |
| Delete line | `Ctrl + K` |
| Go to line | `Ctrl + _` |
| Start of file | `Alt + \` |
| End of file | `Alt + /` |
| Paste | `Ctrl + Shift + V` or right-click (not `Ctrl + V`) |

If nano is closed with `Ctrl + C` mid-save you may see a stray `server.cpp.save` file. It is a backup; it is ignored by `.gitignore`.
