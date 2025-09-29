#ifndef ENGINE_RPM_INTERRUPT_HANDLER_H
#define ENGINE_RPM_INTERRUPT_HANDLER_H

#include <Arduino.h>
#include "config.h"

class EngineRPMInterruptHandler {
private:
    static volatile unsigned long enginePulseCount;
    static volatile unsigned long engineLastPulseTime;
    static EngineRPMInterruptHandler* instance;

    uint8_t gpioPin;
    unsigned long lastCalculationTime;
    float currentRPM;
    unsigned long lastPulseCountSnapshot;
    bool enabled;

    static const unsigned long RPM_CALCULATION_INTERVAL_MS = 1000;
    static const unsigned long RPM_TIMEOUT_MS = 3000;
    static constexpr float MIN_RPM_THRESHOLD = 1.0f;
    static constexpr float MAX_RPM_THRESHOLD = 8000.0f;  // Engine RPM upper limit
    static constexpr float MIN_STABLE_RPM = 300.0f;      // Minimum engine idle RPM

    static void handleInterrupt();

public:
    EngineRPMInterruptHandler(uint8_t pin);

    void begin();
    void update();

    float getRPM() const { return currentRPM; }
    unsigned long getPulseCount() const { return enginePulseCount; }
    bool isReceivingSignal() const;        // Basic pulse detection (for debug)
    bool isValidSignal() const;            // Filtered signal validation (for control)

    void reset();
    void printStatus();
    void setEnabled(bool enable);
    bool isEnabled() const { return enabled; }
};

#endif // ENGINE_RPM_INTERRUPT_HANDLER_H