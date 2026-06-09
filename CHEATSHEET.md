# ESPresense Cheatsheet

Quick reference for building, flashing, git workflow, MQTT/Home Assistant, and diagnostics.

---

## Build & flash

### Prerequisites

PlatformIO is installed via Python on this machine. The `pio` command is not on PATH by default — use `python -m platformio` instead, or add Python Scripts to PATH:

```powershell
$env:Path += ";$env:APPDATA\Python\Python314\Scripts"
```

Permanent PATH entry: `C:\Users\wsergio\AppData\Roaming\Python\Python314\Scripts`

Install PlatformIO (if needed):

```powershell
python -m pip install -U platformio
```

### Build

```powershell
cd C:\Users\wsergio\dev\thetacodeops\experimental\ESPresense
python -m platformio run -e esp32c3      # C3
python -m platformio run -e esp32c6      # C6
python -m platformio run -e esp32c6-cdc  # C6 with USB CDC
```

### Which environment?


| Board                                       | Environment                |
| ------------------------------------------- | -------------------------- |
| ESP32-C3 / SuperMini (UART USB adapter)     | `esp32c3`                  |
| ESP32-C3 USB CDC on boot (single USB cable) | `esp32c3-cdc`              |
| ESP32-C6 / C6 SuperMini (UART USB adapter)  | `esp32c6`                  |
| ESP32-C6 USB CDC on boot (single USB cable) | `esp32c6-cdc`              |
| Classic ESP32                               | `esp32`                    |
| ESP32-S3                                    | `esp32s3` or `esp32s3-cdc` |


### Flash ESP32-C3

Replace `COM6` with your port if different (Device Manager → Ports). See [SERIAL_MONITOR.md](SERIAL_MONITOR.md) for log monitoring.

```powershell
python -m platformio run -e esp32c3 -t upload --upload-port COM6
```

Output firmware: `.pio\build\esp32c3\firmware.bin`

### Flash ESP32-C6

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


### Flash to device (generic)

```powershell
python -m platformio run -e <env> -t upload --upload-port COM6
```

Examples: `esp32c3`, `esp32c3-cdc`, `esp32c6`, `esp32c6-cdc`

Build + upload in one step:

```powershell
python -m platformio run -e esp32c6-cdc -t upload --upload-port COM6
```

### UI build (after changing files under `ui/`)

```powershell
cd ui
npm run build
```

Regenerates C++ headers under `src/`.

---

## MQTT & Home Assistant

### Key MQTT topics


| Topic                                                          | Content                             |
| -------------------------------------------------------------- | ----------------------------------- |
| `espresense/rooms/<room>/status`                               | `online` / offline                  |
| `espresense/rooms/<room>/telemetry`                            | JSON diagnostics (every 15s)        |
| `espresense/rooms/<room>/chip_temperature`                     | On-chip temperature (°C, every 60s) |
| `homeassistant/sensor/espresense_<id>/chip_temperature/config` | HA discovery for chip temp          |


### ESPresense web UI settings


| Setting                     | Default | Notes                          |
| --------------------------- | ------- | ------------------------------ |
| Send to discovery topic     | On      | Required for HA auto-discovery |
| Send to telemetry topic     | On      | Publishes JSON diagnostics     |
| Publish on-chip temperature | On      | Chip Temperature entity        |


After flashing, wait 1–2 minutes. Reload MQTT if needed: **Settings → Devices & services → MQTT → Reload**.

### HA entities (diagnostic)

On the ESPresense device page:

- **Chip Temperature** — on-chip die temp (thermal monitoring)
- **Free Mem** — from telemetry
- **Uptime** — from telemetry
- **Connectivity** — online/offline; telemetry JSON attached as attributes

Chip temp is **die temperature**, not room temperature. Typical range with WiFi + BLE: 40–65°C.

### Telemetry JSON fields (every 15s)

Available on `.../telemetry` and as Connectivity entity attributes:


| Field                                       | Use                     |
| ------------------------------------------- | ----------------------- |
| `rssi`                                      | WiFi signal (dBm)       |
| `freeHeap` / `maxHeap`                      | Memory                  |
| `scanStack` / `loopStack` / `bleStack`      | FreeRTOS stack headroom |
| `fingerprints`                              | BLE devices in memory   |
| `adverts` / `seen` / `queried` / `reported` | BLE pipeline            |
| `failed` / `teleFails` / `reconnectTries`   | Errors / reconnects     |
| `uptime`                                    | Seconds since boot      |
| `firm` / `ver` / `ip`                       | Build & network info    |


### Template sensor from telemetry (no firmware change)

Example — WiFi RSSI from Connectivity attributes:

```yaml
template:
  - sensor:
      - name: "SuperMini WiFi RSSI"
        state: "{{ state_attr('sensor.espresense_<room>_connectivity', 'rssi') }}"
        unit_of_measurement: "dBm"
        device_class: signal_strength
```

Replace `<room>` with your room slug / entity id.

### Remove stale HA entities

If an old entity shows **Unavailable** (e.g. `SuperMini Internal Temperature`):

1. **Settings → Devices & services → Entities** → find entity → **Delete**
2. Or publish empty payload to its MQTT discovery config topic under `homeassistant/sensor/...`
3. Or remove manual `mqtt:` sensor entries from `configuration.yaml`

### Onboard LED control (ESP32-C3 SuperMini)

ESPresense already supports HA LED control via MQTT. On `esp32c3` / `esp32c3-cdc` builds, LED 1 defaults to the SuperMini Plus onboard WS2812 RGB on GPIO 8:

| Setting | Default (C3 firmware) |
|---------|----------------------|
| Pin | GPIO 8 |
| Type | Addressable GRB |
| Count | 1 |
| Control | MQTT (HA on/off; no status blinking) |

Saved **PWM Inverted** configs on GPIO 8 are auto-migrated to **Addressable GRB** at boot (PWM corrupts the shared WS2812).

**HA entity:** **Onboard LED** (`light.espresense_<room>_onboard_led`)

**MQTT topics:**

| Topic | Purpose |
|-------|---------|
| `espresense/rooms/<room>/onboard_led` | State (JSON, retained) |
| `espresense/rooms/<room>/onboard_led/set` | Command — `{"state":"ON"}` or `{"state":"OFF"}` |
| `espresense/rooms/<room>/led_1/set` | Command alias (internal id) |
| `homeassistant/light/espresense_<id>/onboard_led/config` | HA discovery |

Toggle the light in HA to turn the RGB LED on/off (default color blue). Brightness and color are supported via the light entity.

**SuperMini Plus (red PCB):** red power LED (always on), blue LED and WS2812 RGB both on GPIO 8. Firmware drives the WS2812 via **Addressable GRB**; HA **Onboard LED** controls the shared RGB output (default color blue). Do not use PWM on GPIO 8 — it corrupts the WS2812.

**Already-flashed nodes:** old SPIFFS settings may need updating under **Hardware → LED 1**:

- Pin: `8`
- LED Type: `Addressable GRB`
- Count: `1`
- LED Control: `MQTT`

Then reboot. New defaults apply automatically on fresh flash or before hardware settings are first saved.

**Automation example:**

```yaml
action: light.turn_on
target:
  entity_id: light.espresense_<room>_onboard_led
```

---

## Diagnostics reference

### Chip temperature thresholds (ESP32-C3 / C6, WiFi + BLE active)


| Range           | Meaning                               |
| --------------- | ------------------------------------- |
| 40–65°C         | Normal                                |
| 65–75°C         | Warm — check airflow / enclosure      |
| >75°C sustained | Investigate placement or heat buildup |
| 105°C           | Max operating temp (C3/C6 datasheet)  |


### HA automation ideas

- Alert if **Chip Temperature** > 70°C for 10 minutes
- Alert if **Connectivity** goes offline
- Alert if `reconnectTries` or `teleFails` increase (via template from telemetry)
- Alert if `fingerprints` drops to 0 unexpectedly

### Useful metrics to add in firmware (future)


| Metric        | API                    | Why                         |
| ------------- | ---------------------- | --------------------------- |
| WiFi RSSI     | `WiFi.RSSI()`          | Standalone signal entity    |
| WiFi channel  | `WiFi.channel()`       | Interference debug          |
| Min free heap | `ESP.getMinFreeHeap()` | Memory leak detection       |
| CPU frequency | `ESP.getCpuFreqMHz()`  | Power/perf mode             |
| Reset reason  | `esp_reset_reason()`   | Brownout vs watchdog vs OTA |


---

## Troubleshooting


| Problem                                                  | Fix                                                                                                   |
| -------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| `pio` not recognized                                     | Use `python -m platformio` or add Python Scripts to PATH                                              |
| HA entity missing after flash                            | Check discovery + chip temp enabled; reload MQTT; reboot ESP                                          |
| Duplicate temp entities                                  | Delete stale manual/discovery entity; keep **Chip Temperature**                                       |
| Build fails after UI change                              | Run `npm run build` in `ui/`                                                                          |
| Push rejected                                            | `git pull --rebase origin main` then push again                                                       |
| C6 build fails at `bootloader.bin` / esptool `TypeError` | Python 3.14 + old esptool conflict — use PlatformIO IDE extension, or Python 3.11/3.12 for CLI builds |
| C6 upload not detected                                   | Hold BOOT → tap RESET → release BOOT, then upload immediately                                         |
| Wrong C6 env                                             | UART board → `esp32c6`; native USB CDC → `esp32c6-cdc`                                                |
| Onboard LED not in HA / wrong pin                        | Hardware → LED 1: pin 8, PWM Inverted, MQTT; reload MQTT; reboot ESP                                  |
| LED blinks on WiFi/BLE instead of HA control             | Change LED Control from Status to MQTT in hardware settings                                           |


