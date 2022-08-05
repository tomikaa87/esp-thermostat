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

    appConfig.firmwareVersion = VersionNumber{ 1, 5, 0 };

    appConfig.blynk.appToken = Config::Blynk::AppToken;
    appConfig.blynk.serverHostName = Config::Blynk::ServerHostName;
    appConfig.blynk.serverPort = Config::Blynk::ServerPort;

    appConfig.logging.syslog.enabled = true;
    appConfig.logging.syslog.hostName = Config::Logging::SyslogHostName;
    appConfig.logging.syslog.serverHostName = Config::Logging::SyslogServerHost;
    appConfig.logging.syslog.serverPort = Config::Logging::SyslogServerPort;

    appConfig.mqtt.enabled = Config::Mqtt::Enabled;
    if (Config::Mqtt::Enabled) {
        appConfig.mqtt.id = Config::Mqtt::Id;
        appConfig.mqtt.brokerIp = Config::Mqtt::BrokerIp;
        appConfig.mqtt.brokerPort = Config::Mqtt::BrokerPort;
        appConfig.mqtt.user = Config::Mqtt::User;
        appConfig.mqtt.password =  Config::Mqtt::Password;
    }

    appConfig.otaUpdate.updateCheckIntervalMs = 60000;
    appConfig.otaUpdate.updateUrl = Config::OtaUpdate::FirmwareUpdateUrl;
    appConfig.otaUpdate.arduinoOtaPasswordHash = Config::OtaUpdate::ArduinoOtaPasswordHash;

    appConfig.wifi.password = Config::WiFi::Password;
    appConfig.wifi.ssid = Config::WiFi::SSID;

    appConfig.hostName = Config::HostName;

    _thermostat.reset(new Thermostat(appConfig));
}

void loop()
{
    if (_thermostat) {
        _thermostat->task();
    }
}
