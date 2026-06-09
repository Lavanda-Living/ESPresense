#include "SinglePWM.h"

static constexpr uint32_t LEDC_MAX_DUTY = 4095;

SinglePWM::SinglePWM(uint8_t index, ControlType controlType, bool inverted, int pin) : LED(index, controlType) {
    this->inverted = inverted;
    this->pin = pin;
}

void SinglePWM::init() {
    pinMode(pin, OUTPUT);
    if (inverted) {
        digitalWrite(pin, HIGH);
        inited = true;
        return;
    }
#ifdef ARDUINO_V3
    inited = ledcAttach(pin, 5000, 12);
#else
    ledcSetup(LED::getIndex(), 5000, 12);
    ledcAttachPin(pin, getIndex());
    inited = true;
#endif
}

void SinglePWM::update() {
    setDuty(LED::getState() ? LED::getBrightness() : 0);
}

void SinglePWM::setDuty(uint32_t x) {
    if (!inited) init();
    if (!inited) return;

    if (inverted) {
        digitalWrite(pin, x > 0 ? LOW : HIGH);
        return;
    }

    uint32_t duty = x >= 255 ? LEDC_MAX_DUTY : (x <= 0 ? 0 : round(LEDC_MAX_DUTY * pow(10.0, 0.0055 * (x - 255.0))));
#ifdef ARDUINO_V3
    ledcWrite(pin, duty);
#else
    ledcWrite(LED::getIndex(), duty);
#endif
}

void SinglePWM::service() {
}
