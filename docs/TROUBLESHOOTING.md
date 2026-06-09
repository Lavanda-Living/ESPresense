# Troubleshooting

Common issues when building, flashing, and running ESPresense in this fork.

| Problem                                                  | Fix                                                                                                   |
| -------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| `pio` not recognized                                     | Use `python -m platformio` or add Python Scripts to PATH — see [BUILD_AND_FLASH.md](BUILD_AND_FLASH.md) |
| HA entity missing after flash                            | Check discovery + chip temp enabled; reload MQTT; reboot ESP                                          |
| Duplicate temp entities                                  | Delete stale manual/discovery entity; keep **Chip Temperature**                                       |
| Build fails after UI change                              | Run `npm run build` in `ui/`                                                                          |
| Push rejected                                            | `git pull --rebase origin main` then push again                                                       |
| S3/C6 build fails at `bootloader.bin` / esptool `TypeError` | `python -m pip install "click<8.2" --force-reinstall --no-deps` — or Python 3.11/3.12 |
| Atom S3 Lite wrong env / no RGB | Use `esp32s3-cdc` not `m5atom`; LED pin 35 — [BUILD_AND_FLASH.md](BUILD_AND_FLASH.md#flash-m5-atom-s3-lite) |
| C6 upload not detected                                   | Hold BOOT → tap RESET → release BOOT, then upload immediately                                         |
| Wrong C6 env                                             | UART board → `esp32c6`; native USB CDC → `esp32c6-cdc`                                                |
| Onboard LED not in HA / wrong pin                        | Hardware → LED 1: pin 8, Addressable GRB, MQTT; reload MQTT; reboot ESP                               |
| LED blinks on WiFi/BLE instead of HA control             | Change LED Control from Status to MQTT in hardware settings                                           |
| OTA “Host not found”                                     | Enable Arduino OTA; use device IP with `ota-upload.ps1 -Targets`                                      |
| HA light stuck under Controls after category change      | Clear retained discovery topic, reload MQTT, delete entity, reboot ESP — [HOME_ASSISTANT.md](HOME_ASSISTANT.md) |
