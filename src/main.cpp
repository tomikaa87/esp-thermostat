#include "main.h"
#include "Peripherals.h"
#include "PrivateConfig.h"
#include "Thermostat.h"

#include <Arduino.h>

#include <memory>

static std::unique_ptr<Thermostat> _thermostat;

void initializeTempSensor()
{
    // Force a new conversion and wait for the results
    Peripherals::Sensors::MainTemperature::update(true);
    delay(800); // Max. 750 ms for 12-bit
    Peripherals::Sensors::MainTemperature::update();
}

void setup()
{
    initializeTempSensor();

    static ApplicationConfig appConfig;

    appConfig.firmwareVersion = VersionNumber{ 1, 1, 1 };

    appConfig.blynk.appToken = Config::Blynk::AppToken;
    appConfig.blynk.serverHostName = Config::Blynk::ServerHostName;
    appConfig.blynk.serverPort = Config::Blynk::ServerPort;

    appConfig.logging.syslog.enabled = true;
    appConfig.logging.syslog.hostName = Config::Logging::SyslogHostName;
    appConfig.logging.syslog.serverHostName = Config::Logging::SyslogServerHost;
    appConfig.logging.syslog.serverPort = Config::Logging::SyslogServerPort;

    appConfig.otaUpdate.updateCheckIntervalMs = 60000;
    appConfig.otaUpdate.updateUrl = Config::OtaUpdate::FirmwareUpdateUrl;

    appConfig.wifi.password = Config::WiFi::Password;
    appConfig.wifi.ssid = Config::WiFi::SSID;

    _thermostat.reset(new Thermostat(appConfig));
}

void loop()
{
    if (_thermostat) {
        _thermostat->task();
    }
}

#if 0

#include "BlynkHandler.h"
#include "SystemClock.h"
#include "HeatingController.h"
#include "Keypad.h"
#include "main.h"
#include "Settings.h"

#include "display/Display.h"
#include "network/NtpClient.h"

// This header must contain the following configuration values:
// namespace PrivateConfig {
//     static const char* BlynkAppToken = ...;
//     static const char* WiFiSSID = ...;
//     static const char* WiFiPassword = ...;
// }
#include "PrivateConfig.h"

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <Wire.h>

#include "Peripherals.h"

#include <ctime>
#include <memory>

namespace Globals
{
    static std::unique_ptr<Clock> clock;
    static std::unique_ptr<HeatingController> heatingController;
    static std::unique_ptr<BlynkHandler> blynk;
    static std::unique_ptr<NtpClient> ntp;
    static std::unique_ptr<Keypad> keypad;
    static std::unique_ptr<Ui> ui;
}

void ICACHE_RAM_ATTR timer1_isr()
{
    if (Globals::clock) {
        Globals::clock->timerIsr();
    }
}

void setup()
{
    Peripherals::Sensors::MainTemperature::update();



    Serial.begin(115200);

    Wire.begin();
    // Be cautious with 400 kHz, it can produce RTC errors and OLED artifacts
    Wire.setClock(800000);

    auto eeramStatus = Peripherals::Storage::EERAM::getStatus();
    Serial.printf("EERAM status: AM=%u, BP=%u, ASE=%u, EVENT=%u\n",
        eeramStatus.am, eeramStatus.bp, eeramStatus.ase, eeramStatus.event
    );

    Globals::clock.reset(new Clock);
    Globals::heatingController.reset(new HeatingController(*Globals::clock));

    Serial.println("Initializing Display...");
    Display::init();

    Serial.println("Initializing RTC...");

    Serial.println("Loading settings...");
    settings_init(
        [](const uint8_t address) {
            uint8_t data;
            if (Peripherals::Storage::EERAM::read(address, &data, 1) != 1)
                Serial.printf("EERAM read failure at %02xh\n", address);
            Serial.printf("RAM[%02xh] -> %02xh\n", address, data);
            return data;
        },
        [](const uint8_t address, const uint8_t data) {
            Serial.printf("RAM[%02xh] <- %02xh\n", address, data);
            Peripherals::Storage::EERAM::write(address, &data, 1);
        }
    );
    settings_load();

    Serial.println("Initializing keypad...");
    Globals::keypad.reset(new Keypad);

    // Serial.println("Initializing HeatCtl...");
    // heatctl_init();

    Serial.println("Initializing UI...");
    Globals::ui.reset(new Ui);

    connect_wifi();

    Serial.println("Initializing Blynk...");
    Globals::blynk.reset(new BlynkHandler(PrivateConfig::BlynkAppToken));

    Globals::blynk->setActivateBoostCallback([] {
        if (!Globals::heatingController->isBoostActive()) {
            Globals::heatingController->activateBoost();
        } else {
            Globals::heatingController->extendBoost();
        }
    });
    Globals::blynk->setDaytimeTemperatureChangedCallback([](const float value) {
        Globals::heatingController->setDaytimeTemp(value * 10);
    });
    Globals::blynk->setDeactivateBoostCallback([] {
        Globals::heatingController->deactivateBoost();
    });
    Globals::blynk->setDecrementTempCallback([] {
        Globals::heatingController->decTargetTemp();
    });
    Globals::blynk->setIncrementTempCallback([] {
        Globals::heatingController->incTargetTemp();
    });
    Globals::blynk->setModeChangedCallback([](const uint8_t mode) {
        Globals::heatingController->setMode(static_cast<HeatingController::Mode>(mode));
    });
    Globals::blynk->setNightTimeTemperatureChangedCallback([](const float value) {
        Globals::heatingController->setNightTimeTemp(value * 10);
    });
    Globals::blynk->setTargetTemperatureChangedCallback([](const float value) {
        Globals::heatingController->setTargetTemp(value * 10);
    });

    Globals::ntp.reset(new NtpClient(*Globals::clock));

    wifi_screen_set_scan_cb([] {
        return WiFi.scanNetworks();
    });

    wifi_screen_set_read_ssid_cb([](int8_t index, char* ssid) {
        strcpy(ssid, WiFi.SSID(index).c_str());
    });

    wifi_screen_set_is_open_cb([](int8_t index) {
        return WiFi.encryptionType(index) == AUTH_OPEN;
    });
}

void updateBlynk()
{
    Globals::blynk->updateActiveTemperature(Globals::heatingController->targetTemp() / 10.f);
    Globals::blynk->updateBoostRemaining(Globals::heatingController->boostRemaining());
    Globals::blynk->updateCurrentTemperature(Globals::heatingController->currentTemp() / 10.f);
    Globals::blynk->updateDaytimeTemperature(Globals::heatingController->daytimeTemp() / 10.f);
    Globals::blynk->updateIsBoostActive(Globals::heatingController->isBoostActive());
    Globals::blynk->updateIsHeatingActive(Globals::heatingController->isActive());
    Globals::blynk->updateMode(static_cast<uint8_t>(Globals::heatingController->mode()));
    Globals::blynk->updateNightTimeTemperature(Globals::heatingController->nightTimeTemp() / 10.f);

    const auto nt = Globals::heatingController->nextTransition();
    Globals::blynk->updateNextSwitch(static_cast<uint8_t>(nt.state), nt.weekday, nt.hour, nt.minute);
}

void loop()
{
    static auto lastUpdate = 500u;
    static auto lastBlynkUpdate = 1000u;

    Globals::ntp->task();
    Globals::clock->task();

    if (millis() - lastUpdate > 500) {
        Peripherals::Sensors::MainTemperature::update();
        Globals::heatingController->task();
        Globals::ui->update();
        lastUpdate = millis();
    }

    if (millis() - lastBlynkUpdate > 1000u) {
        updateBlynk();
        lastBlynkUpdate = millis();
    }

    const auto pressedKeys = Globals::keypad->scan();
    Globals::ui->handleKeyPress(pressedKeys);

    Globals::blynk->task();
}
#endif