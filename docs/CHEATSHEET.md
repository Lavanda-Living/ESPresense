# ESPresense quick reference

This index replaced the single large cheatsheet. Use the guides below.

| Topic | Guide |
|-------|--------|
| Build, flash, environments, OTA (incl. Atom S3 Lite) | [BUILD_AND_FLASH.md](BUILD_AND_FLASH.md) |
| MQTT, HA entities, telemetry, onboard LED | [HOME_ASSISTANT.md](HOME_ASSISTANT.md) |
| Common problems | [TROUBLESHOOTING.md](TROUBLESHOOTING.md) |
| Serial monitor | [SERIAL_MONITOR.md](SERIAL_MONITOR.md) |
| Fork / upstream sync | [FORK.md](FORK.md) |
| Batch wireless OTA | [`ota-upload.ps1`](../ota-upload.ps1) |

**Fast path (C3 SuperMini):**

```powershell
cd C:\Users\wsergio\dev\thetacodeops\experimental\ESPresense
python -m platformio run -e esp32c3-cdc -t upload --upload-port COM6
```
