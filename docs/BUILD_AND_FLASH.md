# Build & flash

PlatformIO setup, environments, and USB flashing for ESPresense boards.

---

## Prerequisites

PlatformIO is installed via Python on this machine. The `pio` command is not on PATH by default — use `python -m platformio` instead, or add Python Scripts to PATH:

```powershell
$env:Path += ";$env:APPDATA\Python\Python314\Scripts"
```

Permanent PATH entry: `C:\Users\wsergio\AppData\Roaming\Python\Python314\Scripts`

Install PlatformIO (if needed):

```powershell
python -m pip install -U platformio
```

---

## Build

```powershell
cd C:\Users\wsergio\dev\thetacodeops\experimental\ESPresense
python -m platformio run -e esp32c3-cdc   # C3 SuperMini (default for this fork)
python -m platformio run -e esp32c3        # C3 with UART/CH340 USB only
python -m platformio run -e esp32c6-cdc    # C6 with USB CDC
python -m platformio run -e esp32c6        # C6 with UART/CH340 USB only
python -m platformio run -e esp32s3-cdc    # S3 / M5 Atom S3 Lite (USB CDC)
```

---

## Which environment?

| Board | Environment | When to use |
| ----- | ----------- | ----------- |
| **ESP32-C3 SuperMini / Plus** (single USB-C cable) | `esp32c3-cdc` | **Use this** — native USB serial on COM port |
| ESP32-C3 with CH340 / CP2102 USB-UART | `esp32c3` | Separate serial chip, not native USB CDC |
| **ESP32-C6 SuperMini** (single USB-C cable) | `esp32c6-cdc` | Native USB serial |
| ESP32-C6 with CH340 / external UART | `esp32c6` | Separate serial chip |
| Classic ESP32 | `esp32` | |
| **M5 Atom S3 Lite** (USB-C) | `esp32s3-cdc` | Native USB CDC — see below |
| ESP32-S3 (other) | `esp32s3` or `esp32s3-cdc` | UART adapter vs native USB CDC |

---

## `esp32c3` vs `esp32c3-cdc` — same firmware, different USB

Both environments build the **same ESPresense application** (WiFi, BLE, MQTT, HA features). The only compile-time difference is how **USB serial** is wired at boot:

| | `esp32c3` | `esp32c3-cdc` |
| --- | --- | --- |
| USB | UART via CH340/CP2102 (or external adapter) | **Native USB CDC** on the USB-C port |
| Flash / monitor over SuperMini USB-C | Often broken or silent | Works |
| Telemetry `firm` field | `esp32c3` | `esp32c3-cdc` |
| Pick for SuperMini | Wrong choice | **Correct choice** |

**Will both run ESPresense?** Yes — presence, MQTT, and Home Assistant behave the same once flashed.

**Will both flash and log over USB on a SuperMini?** No. SuperMini boards use native USB CDC; use **`esp32c3-cdc`** so upload and `device monitor` work on `COM6`.

If `esp32c3-cdc` already builds, flashes, and shows serial logs, use it for **all** your C3 SuperMinis. Only use `esp32c3` if you know your board has a separate UART chip.

---

## Flash ESP32-C3

Replace `COM6` with your port if different (Device Manager → Ports). See [SERIAL_MONITOR.md](SERIAL_MONITOR.md) for log monitoring.

**SuperMini / SuperMini Plus (recommended):**

```powershell
python -m platformio run -e esp32c3-cdc -t upload --upload-port COM6
```

Output firmware: `.pio\build\esp32c3-cdc\firmware.bin`

**CH340 / UART adapter only:**

```powershell
python -m platformio run -e esp32c3 -t upload --upload-port COM6
```

Output firmware: `.pio\build\esp32c3\firmware.bin`

**If upload fails:** hold **BOOT** → tap **RESET** → release **BOOT** → run upload immediately.

---

## Flash ESP32-C6

The **ChipTemp** change is compatible with ESP32-C6. The C6 has the same on-chip temperature sensor API (`SOC_TEMP_SENSOR_SUPPORTED` + `driver/temperature_sensor.h` on IDF 5.x). After flashing, **Chip Temperature** appears in HA the same way as on C3.

> **Note:** C6 uses a newer PlatformIO platform (Arduino-ESP32 3.2.1 / IDF 5.4). The first build downloads a separate toolchain and takes longer than C3.

**Pick the environment:**

| Your board                    | Environment   | When to use                                       |
| ----------------------------- | ------------- | ------------------------------------------------- |
| C6 DevKitC, UART/CH340 USB    | `esp32c6`     | Separate serial chip, or external USB-UART        |
| C6 SuperMini / native USB CDC | `esp32c6-cdc` | Single USB cable, shows as USB JTAG/serial device |

**Build:**

```powershell
python -m platformio run -e esp32c6
# or for USB CDC boards:
python -m platformio run -e esp32c6-cdc
```

Output firmware: `.pio\build\esp32c6\firmware.bin` (or `esp32c6-cdc`).

**Flash:**

```powershell
python -m platformio run -e esp32c6 -t upload --upload-port COM6
# or:
python -m platformio run -e esp32c6-cdc -t upload --upload-port COM6
```

**Find the COM port (PowerShell):**

```powershell
python -m platformio device list
```

**If upload fails — enter bootloader manually:**

Many C6 boards (especially SuperMini) need BOOT held during connect:

1. Hold **BOOT** (or **B**)
2. Press and release **RESET** (or **R**)
3. Release **BOOT**
4. Run the upload command immediately

**Monitor serial after flash:** see [SERIAL_MONITOR.md](SERIAL_MONITOR.md).

```powershell
python -m platformio device monitor -e esp32c6-cdc --port COM6
```

**C6 vs C3 differences:**

|                | ESP32-C3               | ESP32-C6                    |
| -------------- | ---------------------- | --------------------------- |
| PlatformIO env | `esp32c3`              | `esp32c6`                   |
| Arduino-ESP32  | 2.x (Tasmota platform) | 3.2.1 (pioarduino platform) |
| NimBLE         | v1                     | v2 (`NIMBLE_V2`)            |
| Chip temp      | Supported              | Supported                   |
| HA entity      | Chip Temperature       | Chip Temperature (same)     |

---

## Flash M5 Atom S3 Lite

The Atom S3 Lite is **ESP32-S3** (USB-C, 8 MB flash). Use **`esp32s3-cdc`**, not `m5atom` — that target is for the **original M5 Atom Matrix** (classic ESP32, 25-LED matrix on GPIO 27).

| Spec | Value |
|------|--------|
| RGB | 4× WS2812C on **GPIO 35** (GRB) |
| Button | **GPIO 41** (active low) |
| PlatformIO env | **`esp32s3-cdc`** |

> **Note:** S3 uses Arduino-ESP32 **3.2.1** (same platform family as C6). The first build downloads extra packages and takes longer than C3. If the build fails at `bootloader.bin` with an esptool `TypeError`, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

**Build:**

```powershell
python -m platformio run -e esp32s3-cdc
```

Output firmware: `.pio\build\esp32s3-cdc\firmware.bin`

**Flash** (replace `COM4` with your port — use a data-capable USB-C cable):

```powershell
python -m platformio run -e esp32s3-cdc -t upload --upload-port COM4
```

**Upload-only:**

```powershell
python -m platformio run -e esp32s3-cdc -t upload --upload-port COM4 --no-build
```

**If upload does not connect — download mode** (no separate BOOT button):

1. Hold **reset** for ~2 seconds until the **green LED** turns on
2. Release
3. Run the upload command immediately

**Monitor:**

```powershell
python -m platformio device monitor -e esp32s3-cdc --port COM4
```

Noisy link output (`The system cannot find the path specified`, `espShow` warnings) is usually harmless if upload shows `Writing at 0x...`.

**After first boot — Hardware → LED 1:**

| Setting | Value |
|---------|--------|
| Pin | `35` |
| LED Type | `Addressable GRB` |
| Count | `4` |
| LED Control | `MQTT` (HA) or `Status` (WiFi/BLE indicator) |

Set the **room name** in network settings (`espresense-<room>` hostname) before using OTA.

---

## Flash to device (generic)

```powershell
python -m platformio run -e <env> -t upload --upload-port COM6
```

Examples: `esp32c3-cdc` (C3 SuperMini), `esp32c3`, `esp32c6-cdc`, `esp32c6`, `esp32s3-cdc` (Atom S3 Lite)

Build + upload in one step (C3 SuperMini):

```powershell
python -m platformio run -e esp32c3-cdc -t upload --upload-port COM6
```

---

## UI build (after changing files under `ui/`)

```powershell
cd ui
npm run build
```

Regenerates C++ headers under `src/`.

---

## Wireless OTA

After a node is on Wi‑Fi and running ESPresense (USB flash at least once):

1. Enable **Arduino OTA** on the device web UI or in HA **Configuration**
2. Note hostname `espresense-<room>.local` or the device IP (`.local` often fails on Windows — use IP)
3. Run [`ota-upload.ps1`](../ota-upload.ps1) from the repo root with the matching **environment**

```powershell
# C3 SuperMinis
.\ota-upload.ps1 -Environment esp32c3-cdc -Rooms bedroom,office

# Atom S3 Lite
.\ota-upload.ps1 -Environment esp32s3-cdc -Rooms office

# Skip rebuild; use IP when mDNS fails
.\ota-upload.ps1 -Environment esp32s3-cdc -SkipBuild -Targets 192.168.1.50
```

Use **one environment per script run**. Mixing C3 and S3 nodes means separate invocations with the correct `-Environment` each time. The script uses `espota.py` on port **3232**; your PC must be on the same LAN.
