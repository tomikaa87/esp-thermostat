#pragma once

#include <array>
#include <cstdint>

class HeatingZoneController
{
public:
    /**
     * @brief Value of degrees multiplied by 10.
     */
    using DeciDegrees = uint32_t;

    /**
     * @brief Bit mask of high target temperature (30-minute slots) for 7 days.
     */
    using ScheduleData = std::array<uint8_t, 6 * 7>;

    struct Configuration
    {
        uint32_t overrideTimeoutSeconds{ 120 * 60 };
        uint32_t boostInitialDurationSeconds{ 30 * 60 };
        uint32_t boostExtensionDurationSeconds{ 15 * 60 };
        DeciDegrees heatingOvershoot{};
        DeciDegrees heatingUndershoot{};
        ScheduleData scheduleData{{}};
    };

    enum class Mode
    {
        Off,
        Auto,
        Holiday
    };
    
    explicit HeatingZoneController(Configuration config);

    void updateConfig(Configuration config);

    void setMode(Mode mode);
    [[nodiscard]] Mode mode() const;

    void startOrExtendBoost();
    void stopBoost();
    [[nodiscard]] bool boostActive() const;

    void setHighTargetTemperature(DeciDegrees value);
    void setLowTargetTemperature(DeciDegrees value);
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

    /**
     * @brief Returns the effective target temperature based on the schedule
     * and if there is an override.
     * 
     * @return Temperature 
     */
    [[nodiscard]] DeciDegrees targetTemperature() const;

    /**
     * @brief Checks if heating is requested for the current zone.
     * 
     * @return true 
     * @return false 
     */
    [[nodiscard]] bool callingForHeating() const;

private:
    Configuration _config;

    Mode _mode{ Mode::Off };

    bool _overrideActive{ false };
    DeciDegrees _overrideTemperature{};
};