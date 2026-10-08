# Protocol Reference

Quick reference for the wire format. Full specification: [../docs/PROTOCOL.md](../docs/PROTOCOL.md)

Messages are `|`-separated, newline-terminated text lines.

```text
REGISTER|DEV001|ECU
TELEMETRY|DEV001|28.7|1250|3.30
HEARTBEAT|DEV001
ACK
```

| Type | Direction | Fields |
|---|---|---|
| `REGISTER` | device -> server | device_id, device_type |
| `TELEMETRY` | device -> server | device_id, temperature (C), speed (RPM), voltage (V) |
| `HEARTBEAT` | device -> server | device_id |
| `ACK` | server -> device | none |
