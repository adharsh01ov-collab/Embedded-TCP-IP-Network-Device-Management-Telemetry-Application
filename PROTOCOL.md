# Application-Layer Protocol Specification

Version 1.0

## Transport

- TCP, IPv4
- Server: `127.0.0.1:5000`
- Text encoding: ASCII

## Framing

Every message is a single line terminated by `\n` (LF). The server also tolerates `\r\n`: a trailing `\r` is stripped.

Fields are separated by the pipe character `|`.

```text
<TYPE>|<field1>|<field2>|...\n
```

Because TCP is a byte stream, the receiver must buffer incoming bytes and split on `\n`. A single `recv()` may contain several messages or only part of one.

## Messages

### REGISTER (device -> server)

```text
REGISTER|<device_id>|<device_type>\n
```

| Field | Description | Example |
|---|---|---|
| device_id | Unique device identifier | `DEV001` |
| device_type | Kind of device | `ECU` |

Sent once, immediately after the TCP connection is established.

### TELEMETRY (device -> server)

```text
TELEMETRY|<device_id>|<temperature>|<speed>|<voltage>\n
```

| Field | Unit | Example |
|---|---|---|
| temperature | degrees C | `28.7` |
| speed | RPM | `1250` |
| voltage | V | `3.30` |

Sent every 3 seconds. Values are produced with `std::to_string`, so they appear with six decimal places on the wire (for example `28.700000`).

### HEARTBEAT (device -> server)

```text
HEARTBEAT|<device_id>\n
```

Sent after every second telemetry message (about every 6 seconds).

### ACK (server -> device)

```text
ACK\n
```

Sent by the server after each complete message has been processed.

### Unknown messages

Any message with an unrecognised type is printed by the server as `[UNKNOWN MESSAGE]`; an `ACK` is still returned.

## Sequence

```text
DEVICE                              SERVER
  |-- TCP connect -------------------->|
  |-- REGISTER|DEV001|ECU ------------>|
  |<-------------------------- ACK ----|
  |-- TELEMETRY|DEV001|... ----------->|   (t = 3 s)
  |<-------------------------- ACK ----|
  |-- TELEMETRY|DEV001|... ----------->|   (t = 6 s)
  |<-------------------------- ACK ----|
  |-- HEARTBEAT|DEV001 ---------------->|
  |<-------------------------- ACK ----|
  |           ... repeats ...          |
```

## Termination

Either side closes the socket. The server detects a graceful close when `recv()` returns `0`.

## Known gaps in v1.0

- No field validation or length limits
- No protocol version field
- No checksum (relies on TCP)
- The device does not consume ACKs, so unread ACK data accumulates in its receive buffer
