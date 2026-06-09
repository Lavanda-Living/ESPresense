#pragma once
#ifdef SENSORS
namespace ChipTemp {
    void ConnectToWifi(bool updating);
    void SerialReport();
    bool SendDiscovery();
    void Setup();
    void Loop();
}
#endif
