# Serial USB log monitoring

How to view ESPresense debug output from ESP32 boards over USB serial.

**Default port in this guide:** `COM6` (change if your board uses a different port).

**Baud rate:** `115200` (configured in `platformio.ini`).

---

## Find your COM port

```powershell
python -m platformio device list
```

On Windows you can also check **Device Manager → Ports (COM & LPT)**.

If `COM6` is not listed, use whatever port appears for your board (e.g. `USB Serial Device` or `USB JTAG/serial` on C3/C6 CDC boards).

---

## Open the serial monitor

From the project root:

**ESP32-C3 SuperMini (USB CDC — typical):**

```powershell
cd C:\Users\wsergio\dev\thetacodeops\experimental\ESPresense
python -m platformio device monitor -e esp32c3-cdc --port COM6
```

**ESP32-C3 (UART / CH340 adapter):**

```powershell
python -m platformio device monitor -e esp32c3 --port COM6
```

**ESP32-C6 (USB CDC):**

```powershell
python -m platformio device monitor -e esp32c6-cdc --port COM6
```

**Exit monitor:** `Ctrl+C`

---

## Flash and monitor in one session

Build, upload, then attach the monitor:

```powershell
cd C:\Users\wsergio\dev\thetacodeops\experimental\ESPresense
python -m platformio run -e esp32c3-cdc -t upload --upload-port COM6
python -m platformio device monitor -e esp32c3-cdc --port COM6
```

---

## PlatformIO IDE (Cursor / VS Code)

1. Install the **PlatformIO** extension.
2. Set the upload/monitor port to `COM6` in `platformio.ini` or via the device toolbar.
3. Click the **plug icon** in the status bar (Serial Monitor), or use **PlatformIO → Monitor** under the `esp32c3-cdc` environment.

---

## Other serial terminals

| Tool | Settings |
|------|----------|
| PuTTY | Connection type: **Serial**, port: **COM6**, speed: **115200**, data bits 8, stop bits 1, parity None |
| Arduino IDE | Tools → Serial Monitor, **115200** baud, select **COM6** |

---

## What you should see

After boot, logs typically include:

- Firmware version and room name
- WiFi / MQTT connection status
- `Chip Temp: …` (if chip temperature is enabled)
- BLE scan / fingerprint activity
- Errors and stack traces (decoded when using PlatformIO monitor filters)

PlatformIO applies `esp32_exception_decoder` and `time` filters from `platformio.ini` automatically.

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Port busy / access denied | Close other serial monitors, PuTTY, or upload tools using COM6 |
| No output | Press **RESET** on the board; confirm correct env (`esp32c3-cdc` vs `esp32c3`) |
| Wrong garbled text | Set baud to **115200** |
| COM6 not listed | Try another USB cable (data-capable); reinstall USB drivers; run `device list` again |
| Monitor works, upload fails | Hold **BOOT** → tap **RESET** → release **BOOT**, then upload immediately |

Only one application can use COM6 at a time — stop the monitor before flashing.
