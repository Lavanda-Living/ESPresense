#ifdef SENSORS
#include "ChipTemp.h"

#include "globals.h"
#include "mqtt.h"

#include <HeadlessWiFiSettings.h>

#if CONFIG_IDF_TARGET_ESP32
#include "esp_system.h"
#define CHIP_TEMP_SUPPORTED 1
#define CHIP_TEMP_LEGACY_ESP32 1
#elif defined(SOC_TEMP_SENSOR_SUPPORTED) && SOC_TEMP_SENSOR_SUPPORTED
#define CHIP_TEMP_SUPPORTED 1
#if __has_include("driver/temperature_sensor.h")
#include "driver/temperature_sensor.h"
#define CHIP_TEMP_NEW_API 1
#elif __has_include("driver/temp_sensor.h")
#include "driver/temp_sensor.h"
#define CHIP_TEMP_OLD_API 1
#endif
#endif

namespace ChipTemp {
bool enabled = false;
unsigned long previousMillis = 0;
int sensorInterval = 60000;
bool initialized = false;

#if defined(CHIP_TEMP_NEW_API)
temperature_sensor_handle_t tempHandle = NULL;
#endif

bool readTemperature(float& celsius) {
#if defined(CHIP_TEMP_LEGACY_ESP32)
    celsius = temperatureRead();
    return true;
#elif defined(CHIP_TEMP_NEW_API)
    return temperature_sensor_get_celsius(tempHandle, &celsius) == ESP_OK;
#elif defined(CHIP_TEMP_OLD_API)
    return temp_sensor_read_celsius(&celsius) == ESP_OK;
#else
    return false;
#endif
}

void Setup() {
#if !defined(CHIP_TEMP_SUPPORTED)
    return;
#else
    if (!enabled) return;

#if defined(CHIP_TEMP_LEGACY_ESP32)
    initialized = true;
#elif defined(CHIP_TEMP_NEW_API)
    temperature_sensor_config_t tempConfig = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
    if (temperature_sensor_install(&tempConfig, &tempHandle) == ESP_OK && temperature_sensor_enable(tempHandle) == ESP_OK) {
        initialized = true;
    } else {
        Log.println("[ChipTemp] Failed to initialize on-chip temperature sensor");
    }
#elif defined(CHIP_TEMP_OLD_API)
    temp_sensor_config_t tempConfig = TSENS_CONFIG_DEFAULT();
    if (temp_sensor_set_config(tempConfig) == ESP_OK && temp_sensor_start() == ESP_OK) {
        initialized = true;
    } else {
        Log.println("[ChipTemp] Failed to initialize on-chip temperature sensor");
    }
#endif
#endif
}

void ConnectToWifi(bool updating) {
#if defined(CHIP_TEMP_SUPPORTED)
    enabled = HeadlessWiFiSettings.checkbox("chip_temp", true, "Publish on-chip temperature");
#endif
}

void SerialReport() {
#if defined(CHIP_TEMP_SUPPORTED)
    if (!enabled) return;
    Log.println("Chip Temp:    enabled");
#endif
}

void Loop() {
#if !defined(CHIP_TEMP_SUPPORTED)
    return;
#else
    if (!enabled || !initialized) return;

    if (previousMillis != 0 && millis() - previousMillis < static_cast<unsigned long>(sensorInterval)) return;

    float temperature;
    if (!readTemperature(temperature)) return;

    previousMillis = millis();
    Log.println("Chip Temp: " + String(temperature, 1) + "'C");
    pub((roomsTopic + "/chip_temperature").c_str(), 0, 1, String(temperature, 1).c_str());
#endif
}

bool SendDiscovery() {
#if !defined(CHIP_TEMP_SUPPORTED)
    return true;
#else
    if (!enabled) return true;
    return sendSensorDiscovery("Chip Temperature", EC_DIAGNOSTIC, "temperature", "°C");
#endif
}
}  // namespace ChipTemp
#endif
