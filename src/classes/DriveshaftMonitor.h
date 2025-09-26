#ifndef DRIVESHAFT_MONITOR_H
#define DRIVESHAFT_MONITOR_H

#include "config.h"

class DriveshaftMonitor {
private:
    static volatile unsigned long pulseCount;
    static volatile unsigned long lastPulseTime;
    static DriveshaftMonitor* instance;

    unsigned long lastCalculationTime;
    float currentRPM;
    unsigned long lastPulseCountSnapshot;

    static const unsigned long RPM_CALCULATION_INTERVAL_MS = 1000;
    static const unsigned long RPM_TIMEOUT_MS = 3000;
    static constexpr float MIN_RPM_THRESHOLD = 1.0f;

    static void handleInterrupt();

public:
    DriveshaftMonitor();

    void begin();
    void update();

    float getRPM() const { return currentRPM; }
    unsigned long getPulseCount() const { return pulseCount; }
    bool isReceivingSignal() const;

    void reset();
    void printStatus();
};

#endif // DRIVESHAFT_MONITOR_H