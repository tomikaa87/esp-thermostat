#pragma once

#include <array>
#include <cstdint>
#include <optional>

class HeatingZoneController
{
public:
    /**
     * @brief Value of degrees multiplied by 10.
     */
    using DeciDegrees = int32_t;

    /**
     * @brief Bit mask of high target temperature (30-minute slots) for 7 days.
     */
    using ScheduleData = std::array<uint8_t, 6 * 7>;

    struct Configuration
    {
        uint32_t overrideTimeoutSeconds{ 120 * 60 };
        uint32_t boostInitialDurationSeconds{ 30 * 60 };
        uint32_t boostExtensionDurationSeconds{ 15 * 60 };
        DeciDegrees heatingOvershoot{ 5 };
        DeciDegrees heatingUndershoot{ 5 };
        DeciDegrees holidayModeTemperature{ 180 };
        ScheduleData scheduleData{{}};
    };

    enum class Mode
    {
        Off,
        Auto,
        Holiday
    };

    explicit HeatingZoneController(Configuration config);

    bool updateConfig(Configuration config);

    void updateDateTime(int dayOfWeek, int hour, int minute);

    void setMode(Mode mode);
    [[nodiscard]] Mode mode() const;

    void startOrExtendBoost();
    void stopBoost();
    [[nodiscard]] bool boostActive() const;
    [[nodiscard]] uint32_t boostRemainingSeconds() const;

    /**
     * @brief Inputs a temperature value coming from a sensor.
     *
     * @param value
     */
    void inputTemperature(DeciDegrees value);

    void setHighTargetTemperature(DeciDegrees value);
    [[nodiscard]] DeciDegrees highTargetTemperature() const;

    void setLowTargetTemperature(DeciDegrees value);
    [[nodiscard]] DeciDegrees lowTargetTemperature() const;

    void overrideTargetTemperature(DeciDegrees value);

    /**
     * @brief Resets the target temperature to a value which is based on the
     * schedule and the high/low target.
     */
    void resetTargetTemperature();

    /**
     * @brief Checks if target temperature override is active.
     *
     * @return true
     * @return false
     */
    [[nodiscard]] bool targetTemperatureOverrideActive() const;

    [[nodiscard]] uint32_t targetTemperatureOverrideRemainingSeconds() const;

    /**
     * @brief Returns the effective target temperature based on the schedule
     * and if there is an override.
     *
     * @return Temperature
     */
    [[nodiscard]] std::optional<DeciDegrees> targetTemperature() const;

    /**
     * @brief Checks if heating is requested for the current zone.
     *
     * @return true
     * @return false
     */
    [[nodiscard]] bool callingForHeating() const;

    /**
     * @brief Runs the state machine.
     *
     * @param systemClockDeltaMs Elapsed system time since the last call, in milliseconds
     */
    void task(uint32_t systemClockDeltaMs);

private:
    Configuration _config;

    Mode _mode{ Mode::Off };

    int _scheduleDataDay{};
    int _scheduleDataByte{};
    int _scheduleDataMask{ 1 };

    DeciDegrees _lastInputTemperature{};
    DeciDegrees _highTargetTemperature{};
    DeciDegrees _lowTargetTemperature{};

    DeciDegrees _overrideTemperature{};
    uint32_t _overrideRemainingMs{};

    bool _callForHeatingByTemperature{};

    uint32_t _requestedBoostTimeMs{};

    void updateCallForHeatByTemperature();
    DeciDegrees targetTemperatureBySchedule() const;
};