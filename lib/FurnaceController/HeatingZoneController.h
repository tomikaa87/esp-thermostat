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
     * This structure is directly written into the settings memory.
     * Be cautious when modifying this structure to avoid breaking data layout
     * in existing devices. New fields should be added to the end of this structure.
     * When data should be copied over from an existing field into a new one,
     * or a new field should be initialized with a specific value,
     * be sure to do a settings data version check and do the migration with the help
     * of that.
     */
    using Schedule = std::array<uint8_t, 6 * 7>;

    /**
     * @brief Essential controller configuration data.
     * This structure is directly written into the settings memory.
     * Be cautious when modifying this structure to avoid breaking data layout
     * in existing devices. New fields should be added to the end of this structure.
     * When data should be copied over from an existing field into a new one,
     * or a new field should be initialized with a specific value,
     * be sure to do a settings data version check and do the migration with the help
     * of that.
     */
    struct Configuration
    {
        uint32_t overrideTimeoutSeconds{ 120 * 60 };
        uint32_t boostInitialDurationSeconds{ 30 * 60 };
        uint32_t boostExtensionDurationSeconds{ 15 * 60 };
        uint32_t heatingStartDelaySeconds{ 0 };
        DeciDegrees heatingOvershoot{ 5 };
        DeciDegrees heatingUndershoot{ 5 };
        DeciDegrees holidayModeTemperature{ 180 };
    };

    enum class Mode
    {
        Off,
        Auto,
        Holiday
    };

    /**
     * @brief Controller state data.
     * This structure is directly written into the settings memory.
     * Be cautious when modifying this structure to avoid breaking data layout
     * in existing devices. New fields should be added to the end of this structure.
     * When data should be copied over from an existing field into a new one,
     * or a new field should be initialized with a specific value,
     * be sure to do a settings data version check and do the migration with the help
     * of that.
     */
    struct State
    {
        Mode mode{ Mode::Off };
        DeciDegrees highTargetTemperature{ 220 };
        DeciDegrees lowTargetTemperature{ 220 };

        [[nodiscard]] bool operator<=>(const State&) const = default;
    };

    explicit HeatingZoneController(
        const Configuration& config,
        const Schedule& schedule
    );

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

    [[nodiscard]] DeciDegrees lastInputTemperature() const;

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
     * @brief Tells the library if the window in the current zone is open.
     *
     * @param open True if the window is open which suspends heating
     */
    void setWindowOpened(bool open);

    [[nodiscard]] bool windowOpened() const;

    /**
     * @brief Checks if heating is requested for the current zone.
     *
     * @return true
     * @return false
     */
    [[nodiscard]] bool callingForHeating();

    /**
     * @brief Runs the state machine.
     *
     * @param systemClockDeltaMs Elapsed system time since the last call, in milliseconds
     */
    void task(uint32_t systemClockDeltaMs);

    void loadState(const State& state);
    [[nodiscard]] State saveState();
    [[nodiscard]] bool stateChanged() const;

    void handleFurnaceHeatingChanged(bool heating);

    /**
     * @brief Can be used to check if the lock-out is active after closing the window.
     *
     * @return true If the lock-out is active
     * @return false If the lock-out is inactive
     */
    [[nodiscard]] bool openWindowLockoutActive() const;

    /**
     * @brief Returns the remaining time in milliseconds if the lock-out is active.
     *
     * @return uint32_t Remaining time in milliseconds
     */
    [[nodiscard]] uint32_t openWindowLockoutRemainingMs() const;

    [[nodiscard]] bool startDelayActive() const;

private:
    const Configuration& _config;
    const Schedule& _schedule;

    bool _stateChanged{ false };

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

    bool _furnaceHeating{};

    bool _windowOpen{};
    uint32_t _openWindowLockoutRemainingMs{};

    uint32_t _heatingStartDelayRemainingMs{};

    DeciDegrees targetTemperatureBySchedule() const;
};