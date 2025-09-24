#include "DisplayManager.h"
#include <Wire.h>

DisplayManager::DisplayManager()
	: display(nullptr),
	  lastDisplayUpdate(0),
	  isInitialized(false),
	  currentPage(0),
	  currentGear(1),
	  currentSpeed(0),
	  gearName("N"),
	  servoMoving(false),
	  stepperMoving(false),
	  calibrated(false) {
}

DisplayManager::~DisplayManager() {
    if (display) {
        delete display;
    }
}

bool DisplayManager::begin() {
    Serial.println("Initializing OLED display with Adafruit library...");
    Serial.println("Using default I2C pins (SDA=21, SCL=22)");

    // Create display object (like working project)
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    // Initialize display (like working project)
    if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        delete display;
        display = nullptr;
        return false;
    }

    Serial.println("OLED display initialized successfully!");

    // Configure display settings
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(WHITE);

    isInitialized = true;

    // Show boot screen
    showBootScreen();

    return true;
}

void DisplayManager::update() {
    if (!isInitialized || !display) return;

    unsigned long currentTime = millis();
    if (currentTime - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL_MS) {
        return;
    }

    lastDisplayUpdate = currentTime;

    // Clear display and draw current page
    display->clearDisplay();

    switch (currentPage) {
        case 0:
            drawStatusPage();
            break;
        case 1:
            drawDiagnosticsPage();
            break;
        case 2:
            drawSettingsPage();
            break;
        default:
            currentPage = 0;
            drawStatusPage();
            break;
    }

    display->display();
}

void DisplayManager::drawHeader() {
    if (!display) return;

    // Title bar
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->print("Speedometer v");
    display->println(VERSION_STRING);

    // Draw line under header
    display->drawLine(0, 10, 128, 10, WHITE);
}

void DisplayManager::drawFooter() {
    if (!display) return;

    // Page indicator
    display->setTextSize(1);
    display->setCursor(0, 54);
    String pageText = "Page " + String(currentPage + 1) + "/" + String(MAX_PAGES);
    // Center the text manually (approximate)
    int textWidth = pageText.length() * 6; // Rough character width
    int xPos = (128 - textWidth) / 2;
    display->setCursor(xPos, 54);
    display->print(pageText);
}

void DisplayManager::drawStatusPage() {
    drawHeader();

    // Gear information
    display->setTextSize(1);
    display->setCursor(0, 20);
    display->print("Gear: ");
    display->println(gearName);

    // Speed information
    display->setCursor(0, 38);
    display->print("Speed: ");
    display->print(currentSpeed);
    display->println(" MPH");

    // Status indicators
    display->setTextSize(1);
    String status = "";
    if (servoMoving) status += "S";
    if (stepperMoving) status += "M";
    if (!calibrated) status += "!";

    if (status.length() > 0) {
        // Right-align status text
        int textWidth = status.length() * 6;
        display->setCursor(128 - textWidth, 20);
        display->print(status);
    }

    drawFooter();
}

void DisplayManager::drawDiagnosticsPage() {
    drawHeader();

    display->setTextSize(1);

    // System status
    display->setCursor(0, 16);
    display->print("Calibrated: ");
    display->println(calibrated ? "YES" : "NO");

    display->setCursor(0, 26);
    display->print("Servo: ");
    display->println(servoMoving ? "MOVING" : "IDLE");

    display->setCursor(0, 36);
    display->print("Stepper: ");
    display->println(stepperMoving ? "MOVING" : "IDLE");

    // Memory info
    display->setCursor(0, 46);
    display->print("Free RAM: ");
    display->print(ESP.getFreeHeap());
    display->println("B");

    drawFooter();
}

void DisplayManager::drawSettingsPage() {
    drawHeader();

    display->setTextSize(1);

    // Configuration info
    display->setCursor(0, 16);
    display->println("Target: 1970 MGB");

    display->setCursor(0, 26);
    display->println("Differential: 3.9:1");

    display->setCursor(0, 36);
    display->println("Tire: 165-80R13");

    display->setCursor(0, 46);
    display->print("Uptime: ");
    display->print(millis() / 1000);
    display->println("s");

    drawFooter();
}

void DisplayManager::clear() {
    if (!isInitialized || !display) return;
    display->clearDisplay();
    display->display();
}

void DisplayManager::setBrightness(int brightness) {
    if (!isInitialized || !display) return;
    // Note: Adafruit SSD1306 doesn't have setBrightness, use dim() instead
    if (brightness < 128) {
        display->dim(true);
    } else {
        display->dim(false);
    }
}

void DisplayManager::nextPage() {
    currentPage = (currentPage + 1) % MAX_PAGES;
}

void DisplayManager::previousPage() {
    currentPage = (currentPage - 1 + MAX_PAGES) % MAX_PAGES;
}

void DisplayManager::showBootScreen() {
    if (!isInitialized || !display) return;

    display->clearDisplay();

    // Title - larger text
    display->setTextSize(2);
    String title = "MGB Speedometer";
    int titleWidth = title.length() * 12; // Rough width for size 2
    display->setCursor((128 - titleWidth) / 2, 10);
    display->println(title);

    // Version and status - smaller text
    display->setTextSize(1);
    String version = "Version " + String(VERSION_STRING);
    int versionWidth = version.length() * 6;
    display->setCursor((128 - versionWidth) / 2, 30);
    display->println(version);

    String status = "Initializing...";
    int statusWidth = status.length() * 6;
    display->setCursor((128 - statusWidth) / 2, 42);
    display->println(status);

    display->display();
    delay(2000);
}

void DisplayManager::showCalibrationScreen(const char* status) {
    if (!isInitialized || !display) return;

    display->clearDisplay();

    // Title
    display->setTextSize(1);
    String title = "CALIBRATION";
    int titleWidth = title.length() * 6;
    display->setCursor((128 - titleWidth) / 2, 10);
    display->println(title);

    // Status - larger text
    display->setTextSize(2);
    String statusStr = String(status);
    int statusWidth = statusStr.length() * 12;
    display->setCursor((128 - statusWidth) / 2, 30);
    display->println(statusStr);

    display->display();
}

void DisplayManager::showErrorScreen(const char* error) {
    if (!isInitialized || !display) return;

    display->clearDisplay();

    // Error title
    display->setTextSize(1);
    String title = "ERROR";
    int titleWidth = title.length() * 6;
    display->setCursor((128 - titleWidth) / 2, 10);
    display->println(title);

    // Error message
    String errorStr = String(error);
    int errorWidth = errorStr.length() * 6;
    display->setCursor((128 - errorWidth) / 2, 30);
    display->println(errorStr);

    display->display();
}

void DisplayManager::updateStatus(int gear, int speed, const char* gearName) {
    this->currentGear = gear;
    this->currentSpeed = speed;
    this->gearName = String(gearName);
}

void DisplayManager::updateDiagnostics(bool servoMoving, bool stepperMoving, bool calibrated) {
    this->servoMoving = servoMoving;
    this->stepperMoving = stepperMoving;
    this->calibrated = calibrated;
}

