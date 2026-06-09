# MQTT & Home Assistant

Topics, entities, telemetry, and C3 onboard LED control.

---

## Key MQTT topics

| Topic                                                          | Content                             |
| -------------------------------------------------------------- | ----------------------------------- |
| `espresense/rooms/<room>/status`                               | `online` / offline                  |
| `espresense/rooms/<room>/telemetry`                            | JSON diagnostics (every 15s)        |
| `espresense/rooms/<room>/chip_temperature`                     | On-chip temperature (°C, every 60s) |
| `homeassistant/sensor/espresense_<id>/chip_temperature/config` | HA discovery for chip temp          |

---

## ESPresense web UI settings

| Setting                     | Default | Notes                          |
| --------------------------- | ------- | ------------------------------ |
| Send to discovery topic     | On      | Required for HA auto-discovery |
| Send to telemetry topic     | On      | Publishes JSON diagnostics     |
| Publish on-chip temperature | On      | Chip Temperature entity        |

After flashing, wait 1–2 minutes. Reload MQTT if needed: **Settings → Devices & services → MQTT → Reload**.

---

## HA entities (diagnostic)

On the ESPresense device page:

- **Chip Temperature** — on-chip die temp (thermal monitoring)
- **WiFi RSSI** — signal strength (dBm)
- **WiFi Channel** — current WiFi channel
- **Free Mem** — current free heap (bytes)
- **Min Free Mem** — lowest free heap since boot (memory leak indicator)
- **CPU Frequency** — CPU clock (MHz)
- **Uptime** — seconds since boot
- **Connectivity** — online/offline; telemetry JSON attached as attributes

New diagnostic sensors appear after flash and MQTT discovery (reload MQTT if needed). Updates every 15s with telemetry.

Chip temp is **die temperature**, not room temperature. Typical range with WiFi + BLE: 40–65°C.

---

## Telemetry JSON fields (every 15s)

Available on `.../telemetry` and as Connectivity entity attributes:

| Field                                       | Use                     |
| ------------------------------------------- | ----------------------- |
| `rssi`                                      | WiFi signal (dBm)       |
| `channel`                                   | WiFi channel            |
| `freeHeap` / `maxHeap` / `minFreeHeap`      | Memory (current, max alloc, low-water) |
| `cpuMhz`                                    | CPU frequency           |
| `scanStack` / `loopStack` / `bleStack`      | FreeRTOS stack headroom |
| `fingerprints`                              | BLE devices in memory   |
| `adverts` / `seen` / `queried` / `reported` | BLE pipeline            |
| `failed` / `teleFails` / `reconnectTries`   | Errors / reconnects     |
| `uptime`                                    | Seconds since boot      |
| `firm` / `ver` / `ip`                       | Build & network info    |

---

## Remove stale HA entities

If an old entity shows **Unavailable** (e.g. `SuperMini Internal Temperature`):

1. **Settings → Devices & services → Entities** → find entity → **Delete**
2. Or publish empty payload to its MQTT discovery config topic under `homeassistant/sensor/...`
3. Or remove manual `mqtt:` sensor entries from `configuration.yaml`

For MQTT lights that won’t move to **Configuration** after a firmware update: clear the retained `homeassistant/light/.../config` topic, reload MQTT, delete the entity, reboot the ESP.

---

## Onboard LED control (ESP32-C3 SuperMini)

ESPresense already supports HA LED control via MQTT. On `esp32c3` / `esp32c3-cdc` builds, LED 1 defaults to the SuperMini Plus onboard WS2812 RGB on GPIO 8:

| Setting | Default (C3 firmware) |
|---------|----------------------|
| Pin | GPIO 8 |
| Type | Addressable GRB |
| Count | 1 |
| Control | MQTT (HA on/off; no status blinking) |

Saved **PWM Inverted** configs on GPIO 8 are auto-migrated to **Addressable GRB** at boot (PWM corrupts the shared WS2812).

**HA entity:** **Onboard LED** (`light.espresense_<room>_onboard_led`) — under **Configuration** on the device page (not Controls)

**MQTT topics:**

| Topic | Purpose |
|-------|---------|
| `espresense/rooms/<room>/onboard_led` | State (JSON, retained) |
| `espresense/rooms/<room>/onboard_led/set` | Command — `{"state":"ON"}` or `{"state":"OFF"}` |
| `espresense/rooms/<room>/led_1/set` | Command alias (internal id) |
| `homeassistant/light/espresense_<id>/onboard_led/config` | HA discovery |

Toggle the light in HA to turn the RGB LED on/off (default color blue). Brightness and color are supported via the light entity. On/off state is saved to flash and restored after power cycle.

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

### Possible future metrics

| Metric       | API                  | Why                       |
| ------------ | -------------------- | ------------------------- |
| Reset reason | `esp_reset_reason()` | Brownout vs watchdog vs OTA |
