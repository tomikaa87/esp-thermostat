#include "BlynkHandler.h"
#include "clock.h"
#include "DisplayInitializer.h"
#include "heat_ctl.h"
#include "keypad.h"
#include "main.h"
#include "settings.h"
#include "ui.h"
#include "wifi_screen.h"

#include "display/Display.h"

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
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include "Peripherals.h"

#include <ctime>
#include <memory>

static DisplayInitializer* display = nullptr;
static BlynkHandler* blynk = nullptr;
static NTPClient* ntp = nullptr;
static std::unique_ptr<Keypad> keypad;

static constexpr auto LocalTimeOffsetMinutes = 60;
static constexpr auto LocalTimeDstOffsetMinutes = 60;

static void syncRtc();

void ICACHE_RAM_ATTR timer1_isr()
{
    static int counter = 0;

    if (++counter == 3125) {
        counter = 0;
        ++clock_epoch;
    }
}

void connect_wifi()
{
    Serial.printf("Connecting to %s", PrivateConfig::WiFiSSID);

    WiFi.mode(WIFI_STA);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    WiFi.setOutputPower(20.5);

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(PrivateConfig::WiFiSSID, PrivateConfig::WiFiPassword);
        // if (pass && strlen(pass)) {
        //     WiFi.begin(ssid, pass);
        // } else {
        //     WiFi.begin(ssid);
        // }
    }

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConnected to WiFi");
}

void setup()
{
    Peripherals::Sensors::MainTemperature::update();

    timer1_isr_init();
    timer1_attachInterrupt(timer1_isr);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
    timer1_write(100);

    Serial.begin(115200);

    Wire.begin();
    // Be cautious with 400 kHz, it can produce RTC errors and OLED artifacts
    Wire.setClock(200000);

    auto eeramStatus = Peripherals::Storage::EERAM::getStatus();
    Serial.printf("EERAM status: AM=%u, BP=%u, ASE=%u, EVENT=%u\n",
        eeramStatus.am, eeramStatus.bp, eeramStatus.ase, eeramStatus.event
    );
    eeramStatus.value = 0;
    eeramStatus.ase = 1;
    Peripherals::Storage::EERAM::setStatus(eeramStatus);
    eeramStatus = Peripherals::Storage::EERAM::getStatus();
    Serial.printf("EERAM status: AM=%u, BP=%u, ASE=%u, EVENT=%u\n",
        eeramStatus.am, eeramStatus.bp, eeramStatus.ase, eeramStatus.event
    );

    Display::init();

    Serial.println("Initializing DisplayInitializer...");
    static DisplayInitializer sDisplay;
    display = &sDisplay;

    Serial.println("Initializing RTC...");

    using rtc = Peripherals::Clock::Rtc;
    if (rtc::getPowerFailFlag()) {
        Serial.println("WARNING: power failure detected");
        rtc::clearPowerFailFlag();
    }

    rtc::setBatteryEnabled(true);
    rtc::setOscillatorEnabled(true);

    if (!rtc::isOscillatorRunning()) {
        Serial.println("WARNING: RTC oscillator has stopped");
    }

    syncRtc();

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
    keypad.reset(new Keypad);

    Serial.println("Initializing HeatCtl...");
    heatctl_init();

    Serial.println("Initializing UI...");
    ui_init();

    connect_wifi();

    Serial.println("Initializing Blynk...");
    static BlynkHandler blynkHandler{
        PrivateConfig::BlynkAppToken
        // PrivateConfig::WiFiSSID,
        // PrivateConfig::WiFiPassword
    };
    blynk = &blynkHandler;

    blynkHandler.setActivateBoostCallback([] {
        if (!heatctl_is_boost_active())
            heatctl_activate_boost();
        else
            heatctl_extend_boost();
    });
    blynkHandler.setDaytimeTemperatureChangedCallback([](const float value) { heatctl_set_daytime_temp(value * 10); });
    blynkHandler.setDeactivateBoostCallback(heatctl_deactivate_boost);
    blynkHandler.setDecrementTempCallback(heatctl_dec_target_temp);
    blynkHandler.setIncrementTempCallback(heatctl_inc_target_temp);
    blynkHandler.setModeChangedCallback([](const uint8_t mode) { heatctl_set_mode(static_cast<heatctl_mode_t>(mode)); });
    blynkHandler.setNightTimeTemperatureChangedCallback([](const float value) { heatctl_set_night_time_temp(value * 10); });
    blynkHandler.setTargetTemperatureChangedCallback([](const float value) { heatctl_set_target_temp(value * 10); });

    // FIXME this is a poor NTP client implementation which blocks the execution while it waits for response.
    // It must be replaced with a non-blocking one that has more sophisticated error handling.
    Serial.println("Initializing NTP Client...");
    static WiFiUDP udpSocket;
    static NTPClient ntpCli{ udpSocket, "europe.pool.ntp.org" };
    ntp = &ntpCli;
    ntpCli.begin();
    ntpCli.forceUpdate();

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

bool isDst(const time_t t)
{
    const auto tm = gmtime(&t);

    // Before March or after October
    if (tm->tm_mon < 2 || tm->tm_mon > 9)
        return false;

    // After March and before October
    if (tm->tm_mon > 2 && tm->tm_mon < 9)
        return true;

    const auto previousSunday = tm->tm_mday - tm->tm_wday;

    // Sunday after March 25th
    if (tm->tm_mon == 2)
        return previousSunday >= 25;

    // Sunday before October 25th
    if (tm->tm_mon == 9)
        return previousSunday < 25;

    // This should never happen
    return false;
}

void setClockEpoch(const time_t utc)
{
    clock_epoch = utc + LocalTimeOffsetMinutes * 60;

    if (isDst(clock_epoch))
        clock_epoch += LocalTimeDstOffsetMinutes * 60;
}

void updateClock()
{
    const time_t utc = ntp->getEpochTime();

    setClockEpoch(utc);

    const auto tm = gmtime(&utc);

    using rtc = Peripherals::Clock::Rtc;
    const rtc::DateTime dt{
        tm->tm_year - 100,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_wday + 1,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    };
    rtc::setDateTime(dt);

    Serial.printf("NTP synchronization finished. Current time (UTC): %d-%02d-%02d %d:%02d:%02d\n",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    );
}

static void syncRtc()
{
    using rtc = Peripherals::Clock::Rtc;
    const auto dt = rtc::getDateTime();

    struct tm rtcTm;
    rtcTm.tm_hour = dt.hours;
    rtcTm.tm_min = dt.minutes;
    rtcTm.tm_sec = dt.seconds;
    rtcTm.tm_mday = dt.date;
    rtcTm.tm_mon = dt.month - 1;     // DS1307: 1-12, C: 0-11
    rtcTm.tm_year = dt.year + 100;   // DS1307: 0-99, C: 1900 + value

    Serial.printf("RTC time (UTC): %d-%02d-%02d %d:%02d:%02d\n",
        rtcTm.tm_year + 1900,
        rtcTm.tm_mon + 1,
        rtcTm.tm_mday,
        rtcTm.tm_hour,
        rtcTm.tm_min,
        rtcTm.tm_sec
    );

    setClockEpoch(mktime(&rtcTm));
}

void updateBlynk()
{
    blynk->updateActiveTemperature(heatctl_target_temp() / 10.f);
    blynk->updateBoostRemaining(heatctl_boost_remaining_secs());
    blynk->updateCurrentTemperature(heatctl_current_temp() / 10.f);
    blynk->updateDaytimeTemperature(heatctl_daytime_temp() / 10.f);
    blynk->updateIsBoostActive(heatctl_is_boost_active());
    blynk->updateIsHeatingActive(heatctl_is_active());
    blynk->updateMode(heatctl_mode());
    blynk->updateNightTimeTemperature(heatctl_night_time_temp() / 10.f);
    
    const auto ns = heatctl_next_state();
    blynk->updateNextSwitch(ns.state, ns.weekday, ns.hour, ns.minute);
}

void loop()
{
    static const auto NtpSyncInterval = 10 * 24 * 60 * 60 * 1000ul;

    static auto lastUpdate = 500u;
    static auto lastRtcSync = 10000u;
    static auto lastNtpSync = NtpSyncInterval;
    static auto lastBlynkUpdate = 1000u;

    if (millis() - lastUpdate > 500) {
        // ds18x20_update();
        Peripherals::Sensors::MainTemperature::update();
        heatctl_task();
        ui_update();
        lastUpdate = millis();
    }

    if (millis() - lastBlynkUpdate > 1000u) {
        updateBlynk();
        lastBlynkUpdate = millis();
    }

    if (millis() - lastRtcSync > 10000) {
        syncRtc();
        lastRtcSync = millis();
    }
    
    if (millis() - lastNtpSync >= NtpSyncInterval)
    {
        if (ntp->forceUpdate())
        {
            lastNtpSync = millis();
            updateClock();
        }
        else
        {
            // Try again 10 seconds later
            lastNtpSync = millis() - NtpSyncInterval + 10000ul;
        }
    }

    const auto pressedKeys = keypad->scan();
    ui_handle_keys(pressedKeys);

    blynk->task();
}
