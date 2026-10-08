# Roadmap

## v1.0 (current)

- [x] TCP connection (Winsock2)
- [x] Device registration
- [x] Telemetry transmission
- [x] Heartbeat transmission
- [x] Newline message framing
- [x] ACK responses
- [x] Error handling and disconnect detection

## Next steps

### Phase 0: Small fixes
- [ ] Have the device read and verify the server's `ACK`
- [ ] Replace deprecated `inet_addr()` with `inet_pton()`
- [ ] Format telemetry with fixed precision (`28.7` instead of `28.700000`)
- [ ] Load IP and port from command-line arguments

### Phase 1: Multiple devices
```text
DEV001 ----\
DEV002 -----+----> Device Manager
DEV003 ----/
```
One thread per client, or a single-threaded `select()` loop.

### Phase 2: Device state table
```text
Device ID    Status     Last Heartbeat
DEV001       ONLINE     2 sec ago
DEV002       ONLINE     1 sec ago
DEV003       OFFLINE    15 sec ago
```

### Phase 3: Heartbeat timeout
Mark a device OFFLINE when no heartbeat arrives within a configured window.

### Phase 4: Automatic reconnection
Device retries with backoff after a connection failure.

### Phase 5: Command and control
```text
START|DEV001
STOP|DEV001
RESET|DEV001
SET_SPEED|DEV001|2000
```

### Phase 6: Wireshark analysis
Capture on the loopback adapter and document the three-way handshake, data segments, ACKs, FIN teardown and retransmissions.

### Phase 7: Real embedded hardware
Replace the simulator with an actual device over Ethernet.

```text
Embedded ECU --(Ethernet / TCP/IP)--> Device Manager --> Monitoring application
```
