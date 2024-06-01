#include "main.h"
#include "FurnaceController.h"
#include "Peripherals.h"
#include "PrivateConfig.h"

#include "ui/Ui.h"

#include <Arduino.h>

#include <memory>

namespace
{
    ApplicationConfig appConfig;
    FurnaceController* furnaceController{};
    UI::UIController* uiController{};
}

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

    appConfig.firmwareVersion = VersionNumber{ 1, 6, 0 };

#ifndef TEST_BUILD
    appConfig.logging.syslog.enabled = true;
    appConfig.logging.syslog.hostName = Config::Logging::SyslogHostName;
    appConfig.logging.syslog.serverHostName = Config::Logging::SyslogServerHost;
    appConfig.logging.syslog.serverPort = Config::Logging::SyslogServerPort;
#endif

    appConfig.mqtt.enabled = Config::Mqtt::Enabled;
    if (Config::Mqtt::Enabled) {
        appConfig.mqtt.id = Config::Mqtt::Id;
        appConfig.mqtt.brokerIp = Config::Mqtt::BrokerIp;
        appConfig.mqtt.brokerPort = Config::Mqtt::BrokerPort;
        appConfig.mqtt.user = Config::Mqtt::User;
        appConfig.mqtt.password =  Config::Mqtt::Password;
    }

#ifdef IOT_ENABLE_HTTP_OTA_UPDATE
    appConfig.otaUpdate.updateCheckIntervalMs = 60000;
    appConfig.otaUpdate.updateUrl = Config::OtaUpdate::FirmwareUpdateUrl;
#endif
    appConfig.otaUpdate.arduinoOtaPasswordHash = Config::OtaUpdate::ArduinoOtaPasswordHash;

    appConfig.wifi.password = Config::WiFi::Password;
    appConfig.wifi.ssid = Config::WiFi::SSID;

    appConfig.hostName = Config::HostName;

    // furnaceController = [] {
    //     static FurnaceController controller{ appConfig };
    //     return &controller;
    // }();

    uiController = [] {
        static UI::UIController controller;
        return &controller;
    }();
}

void loop()
{
    // furnaceController->task();
    uiController->task();
}
