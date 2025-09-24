#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "version.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // No reset pin (like working project)

class DisplayManager {
private:
    Adafruit_SSD1306* display;

    // Display update timing
    static const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 250;  // Update every 250ms
    unsigned long lastDisplayUpdate;

    // Display state tracking
    bool isInitialized;
    int currentPage;
    static const int MAX_PAGES = 3;  // Status, Diagnostics, Settings

    // Display content state
    int currentGear;
    int currentSpeed;
    String gearName;
    bool servoMoving;
    bool stepperMoving;
    bool calibrated;

    // Content helpers
    void drawStatusPage();
    void drawDiagnosticsPage();
    void drawSettingsPage();
    void drawHeader();
    void drawFooter();

public:
    DisplayManager();
    ~DisplayManager();

    // Initialization
    bool begin();

    // Update method - call this regularly in main loop
    void update();

    // Display control
    void clear();
    void setBrightness(int brightness);  // 0-255
    void nextPage();
    void previousPage();
    void showBootScreen();
    void showCalibrationScreen(const char* status);
    void showErrorScreen(const char* error);

    // Content updates
    void updateStatus(int gear, int speed, const char* gearName);
    void updateDiagnostics(bool servoMoving, bool stepperMoving, bool calibrated);

    // Getters
    bool isDisplayInitialized() const { return isInitialized; }
    int getCurrentPage() const { return currentPage; }
};

#endif // DISPLAY_MANAGER_H