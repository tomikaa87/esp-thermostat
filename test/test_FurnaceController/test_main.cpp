#include <gtest/gtest.h>

#include <HeatingZoneController.h>

#include <optional>

#pragma region Utilities

namespace TestUtils
{
    HeatingZoneController::Schedule generateAllHighSchedule()
    {
        HeatingZoneController::Schedule schedule{{}};
        for (auto& byte : schedule) {
            byte = 0xFF;
        }
        return schedule;
    }

    HeatingZoneController::Schedule generateHighScheduleForEveryFirstHalfHour()
    {
        HeatingZoneController::Schedule schedule{{}};
        for (auto& byte : schedule) {
            byte = 0b01010101;
        }
        return schedule;
    }

    HeatingZoneController::Schedule generateHighScheduleForEverySecondHalfHour()
    {
        HeatingZoneController::Schedule schedule{{}};
        for (auto& byte : schedule) {
            byte = 0b10101010;
        }
        return schedule;
    }
}

std::ostream& operator<<(std::ostream& str, const HeatingZoneController::Mode mode)
{
    switch (mode) {
        case HeatingZoneController::Mode::Off:
            str << "Off";
            break;
        case HeatingZoneController::Mode::Auto:
            str << "Auto";
            break;
        case HeatingZoneController::Mode::Holiday:
            str << "Holiday";
            break;
    }
    return str;
}

struct InputParams
{
    HeatingZoneController::Mode mode{ HeatingZoneController::Mode::Off };
    HeatingZoneController::DeciDegrees highTargetTemperature{ 230 };
    HeatingZoneController::DeciDegrees lowTargetTemperature{ 210 };
    HeatingZoneController::DeciDegrees holidayTargetTemperature{ 180 };
    HeatingZoneController::DeciDegrees inputTemperature{ 220 };

    enum class Boost
    {
        Stopped,
        Started,
        Extended
    } boost{ Boost::Stopped };

    bool expectedCallingForHeating{ false };
    bool expectedBoostActive{ false };
    std::optional<HeatingZoneController::DeciDegrees> expectedTargetTemperature;

    std::optional<uint32_t> taskSystemClockMillis;

    HeatingZoneController::Schedule scheduleData{};
};

std::ostream& operator<<(std::ostream& str, const InputParams::Boost mode)
{
    switch (mode) {
        case InputParams::Boost::Stopped:
            str << "Stopped";
            break;
        case InputParams::Boost::Started:
            str << "Started";
            break;
        case InputParams::Boost::Extended:
            str << "Extended";
            break;
    }
    return str;
}

std::ostream& operator<<(std::ostream& str, const InputParams& p)
{
    str << "{";
    str << "mode=" << p.mode;
    str << ",high=" << p.highTargetTemperature;
    str << ",low=" << p.lowTargetTemperature;
    str << ",holiday=" << p.holidayTargetTemperature;
    str << ",input=" << p.inputTemperature;
    str << ",boost=" << p.boost;
    str << ",heat=" << p.expectedCallingForHeating;
    str << ",boost=" << p.expectedBoostActive;
    if (p.expectedTargetTemperature.has_value()) {
        str << ",target=" << p.expectedTargetTemperature.value();
    }
    if (p.taskSystemClockMillis.has_value()) {
        str << ",taskMillis=" << p.taskSystemClockMillis.value();
    }
    str << "}";
    return str;
}

#pragma endregion

#pragma region Long-named tests, should be cleaned up

/*
    Test case naming:
        Mode<mode>_Schedule<schedule>_Boost<boost>_Heating<heating>_<details>

        mode := Off | Auto | Holiday
        schedule := AllLow | AllHigh
        boost := Stopped | Started | Extended
        heating := Idle | Calling
        details: Extra info if needed
*/

TEST(HeatingZoneController, ModeOff_ScheduleAllLow_BoostStopped_HeatingIdle_InitialState)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    EXPECT_FALSE(controller.boostActive());
    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_EQ(controller.mode(), HeatingZoneController::Mode::Off);
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_FALSE(controller.targetTemperature().has_value());
}

TEST(HeatingZoneController, ModeAuto_ScheduleAllLow_BoostStopped_HeatingIdle_LowTargetTemp)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    EXPECT_EQ(controller.targetTemperature(), 210);
}

TEST(HeatingZoneController, ModeAuto_ScheduleAllHigh_BoostStopped_HeatingIdle_HighTargetTemp)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    EXPECT_EQ(controller.targetTemperature(), 230);
}

TEST(HeatingZoneController, ModeOff_ScheduleAllLow_BoostStopped_HeatingIdle)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(220);

    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllHigh_BoostStopped_HeatingIdle)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(200);

    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllHigh_BoostStarted_HeatingCalling)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(240);

    controller.startOrExtendBoost();

    EXPECT_TRUE(controller.callingForHeating());
    EXPECT_TRUE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllHigh_BoostStopped_HeatingIdle_BoostStartedAndStopped)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(240);

    controller.startOrExtendBoost();
    controller.stopBoost();

    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllLow_BoostStopped_HeatingIdle_AfterInitialBoostDuration)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    EXPECT_FALSE(controller.boostActive());

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_EQ(
        controller.boostRemainingSeconds(),
        HeatingZoneController::Configuration{}.boostInitialDurationSeconds
    );

    controller.task(controller.boostRemainingSeconds() * 1000);
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllLow_BoostStopped_HeatingIdle_AfterExtendedBoostDuration)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    EXPECT_FALSE(controller.boostActive());

    controller.startOrExtendBoost();
    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());

    controller.task(HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000);
    EXPECT_TRUE(controller.boostActive());
    EXPECT_EQ(
        controller.boostRemainingSeconds(),
        HeatingZoneController::Configuration{}.boostExtensionDurationSeconds
    );

    controller.task(HeatingZoneController::Configuration{}.boostExtensionDurationSeconds * 1000);
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeHoliday_ScheduleAllLow_BoostStopped_HeatingDependsOnTargetTemperature)
{
    HeatingZoneController::Configuration config{
        .holidayModeTemperature = 150
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.setMode(HeatingZoneController::Mode::Holiday);

    EXPECT_EQ(controller.targetTemperature(), 150);

    controller.inputTemperature(140);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(160);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, ModeHoliday_ScheduleAllHigh_BoostStopped_HeatingDependsOnTargetTemperature)
{
    HeatingZoneController::Configuration config{
        .holidayModeTemperature = 150
    };
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.setMode(HeatingZoneController::Mode::Holiday);

    EXPECT_EQ(controller.targetTemperature(), 150);
}

TEST(HeatingZoneController, ModeHoliday_ScheduleAllLow_BoostStarted_HeatingCalling)
{
    HeatingZoneController::Configuration config{
        .holidayModeTemperature = 150
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.setMode(HeatingZoneController::Mode::Holiday);
    controller.inputTemperature(220);

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, ModeHoliday_ScheduleAllLow_BoostExtended_HeatingCalling)
{
    HeatingZoneController::Configuration config{
        .holidayModeTemperature = 150
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.setMode(HeatingZoneController::Mode::Holiday);
    controller.inputTemperature(220);

    controller.startOrExtendBoost();
    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());
}

#pragma endregion

#pragma region General tests

TEST(HeatingZoneController, HighTargetTemperatureChangeControlsHeating)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.inputTemperature(240);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    EXPECT_FALSE(controller.callingForHeating());

    controller.setHighTargetTemperature(250);
    EXPECT_TRUE(controller.callingForHeating());

    controller.setHighTargetTemperature(230);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, HighTargetTemperatureOvershoot)
{
    HeatingZoneController::Configuration config{
        .heatingOvershoot = 5
    };
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(200);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(230);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(230 + config.heatingOvershoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, HighTargetTemperatureUndershoot)
{
    HeatingZoneController::Configuration config{
        .heatingUndershoot = 5
    };
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);

    controller.inputTemperature(250);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(230);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(230 - config.heatingUndershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, LowTargetTemperatureChangeControlsHeating)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(220);
    EXPECT_FALSE(controller.callingForHeating());

    controller.setLowTargetTemperature(230);
    EXPECT_TRUE(controller.callingForHeating());

    controller.setLowTargetTemperature(210);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, LowTargetTemperatureOvershoot)
{
    HeatingZoneController::Configuration config{
        .heatingOvershoot = 5
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    EXPECT_EQ(controller.targetTemperature(), 210);

    controller.inputTemperature(200);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(210);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(210 + config.heatingOvershoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, LowTargetTemperatureUndershoot)
{
    HeatingZoneController::Configuration config{
        .heatingUndershoot = 5
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    EXPECT_EQ(controller.targetTemperature(), 210);

    controller.inputTemperature(230);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(210);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(210 - config.heatingUndershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, HolidayTargetTemperatureOvershoot)
{
    HeatingZoneController::Configuration config{
        .heatingOvershoot = 5,
        .holidayModeTemperature = 180
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Holiday);
    EXPECT_EQ(controller.targetTemperature(), config.holidayModeTemperature);

    controller.inputTemperature(config.holidayModeTemperature - 20);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(config.holidayModeTemperature);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(config.holidayModeTemperature + config.heatingOvershoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, HolidayTargetTemperatureUndershoot)
{
    HeatingZoneController::Configuration config{
        .heatingUndershoot = 5,
        .holidayModeTemperature = 180
    };
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Holiday);
    EXPECT_EQ(controller.targetTemperature(), config.holidayModeTemperature);

    controller.inputTemperature(config.holidayModeTemperature + 20);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(config.holidayModeTemperature);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(config.holidayModeTemperature - config.heatingUndershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, OffModeReportsNoTargetTemperature)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);

    EXPECT_EQ(controller.targetTemperature(), std::nullopt);
}

TEST(HeatingZoneController, OffModeDoesNotCallForHeatingAutomatically)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());

    controller.overrideTargetTemperature(250);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, OffModeDisablesOverride)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.overrideTargetTemperature(250);
    controller.inputTemperature(100);

    EXPECT_EQ(controller.targetTemperatureOverrideRemainingSeconds(), 0);
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToOffModeDisablesOverride)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);

    controller.overrideTargetTemperature(250);
    controller.inputTemperature(100);

    controller.setMode(HeatingZoneController::Mode::Off);

    EXPECT_EQ(controller.targetTemperatureOverrideRemainingSeconds(), 0);
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToOffModeKeepsBoostRunning)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Off);

    EXPECT_TRUE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToOffModeTurnsOffHeating)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);

    controller.setLowTargetTemperature(210);
    controller.inputTemperature(100);

    EXPECT_TRUE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Off);

    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToAutoModeTurnsOnHeating)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.setLowTargetTemperature(210);
    controller.inputTemperature(100);

    EXPECT_FALSE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Auto);

    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToAutoModeTurnsOnHeatingWithHighTarget)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.setLowTargetTemperature(210);
    controller.setHighTargetTemperature(230);
    controller.inputTemperature(220);

    EXPECT_FALSE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Auto);

    EXPECT_EQ(controller.targetTemperature(), 230);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToHolidayModeTurnsOnHeating)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.inputTemperature(100);

    EXPECT_FALSE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Holiday);

    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, AlwaysHighTargetTemperatureForAllHighSchedule)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateAllHighSchedule()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    for (auto dayOfWeek = 0; dayOfWeek < 7; ++dayOfWeek) {
        for (auto hour = 0; hour < 24; ++hour) {
            for (auto minute = 0; minute < 60; ++minute) {
                controller.updateDateTime(dayOfWeek, hour, minute);

                EXPECT_EQ(controller.targetTemperature(), 230);

                if (controller.targetTemperature() != 230) {
                    std::cout
                        << "dayOfWeek=" << dayOfWeek
                        << ",hour=" << hour
                        << ",minute=" << minute
                        << '\n';
                }
            }
        }
    }
}

TEST(HeatingZoneController, AlwaysLowTargetTemperatureForAllLowSchedule)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    for (auto dayOfWeek = 0; dayOfWeek < 7; ++dayOfWeek) {
        for (auto hour = 0; hour < 24; ++hour) {
            for (auto minute = 0; minute < 60; ++minute) {
                controller.updateDateTime(dayOfWeek, hour, minute);

                EXPECT_EQ(controller.targetTemperature(), 210);

                if (controller.targetTemperature() != 210) {
                    std::cout
                        << "dayOfWeek=" << dayOfWeek
                        << ",hour=" << hour
                        << ",minute=" << minute
                        << '\n';
                }
            }
        }
    }
}

TEST(HeatingZoneController, TargetTemperatureForAlternatingSchedule1)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateHighScheduleForEveryFirstHalfHour()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(220);

    for (auto dayOfWeek = 0; dayOfWeek < 7; ++dayOfWeek) {
        for (auto hour = 0; hour < 24; ++hour) {
            for (auto minute = 0; minute < 60; ++minute) {
                controller.updateDateTime(dayOfWeek, hour, minute);

                bool expectFailed{ false };

                if (minute < 30) {
                    EXPECT_EQ(controller.targetTemperature(), 230);
                    expectFailed = controller.targetTemperature() != 230;

                    EXPECT_TRUE(controller.callingForHeating());
                } else {
                    EXPECT_EQ(controller.targetTemperature(), 210);
                    expectFailed = controller.targetTemperature() != 210;

                    EXPECT_FALSE(controller.callingForHeating());
                }

                if (expectFailed) {
                    std::cout
                        << "dayOfWeek=" << dayOfWeek
                        << ",hour=" << hour
                        << ",minute=" << minute
                        << '\n';
                }
            }
        }
    }
}

TEST(HeatingZoneController, TargetTemperatureForAlternatingSchedule2)
{
    HeatingZoneController::Configuration config{};
    HeatingZoneController::Schedule schedule{
        TestUtils::generateHighScheduleForEverySecondHalfHour()
    };
    HeatingZoneController controller{ config, schedule };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(220);

    for (auto dayOfWeek = 0; dayOfWeek < 7; ++dayOfWeek) {
        for (auto hour = 0; hour < 24; ++hour) {
            for (auto minute = 0; minute < 60; ++minute) {
                controller.updateDateTime(dayOfWeek, hour, minute);

                bool expectFailed{ false };

                if (minute < 30) {
                    EXPECT_EQ(controller.targetTemperature(), 210);
                    expectFailed = controller.targetTemperature() != 210;

                    EXPECT_FALSE(controller.callingForHeating());
                } else {
                    EXPECT_EQ(controller.targetTemperature(), 230);
                    expectFailed = controller.targetTemperature() != 230;

                    EXPECT_TRUE(controller.callingForHeating());
                }

                if (expectFailed) {
                    std::cout
                        << "dayOfWeek=" << dayOfWeek
                        << ",hour=" << hour
                        << ",minute=" << minute
                        << '\n';
                }
            }
        }
    }
}

#pragma endregion

#pragma region Tests for all active modes

class ActiveModeTest
    : public ::testing::TestWithParam<HeatingZoneController::Mode>
{
public:
    ActiveModeTest()
        : config{
            .heatingOvershoot = 5,
            .heatingUndershoot = 5
        }
        , controller{ config, schedule }
    {
        controller.setMode(GetParam());
    }

    HeatingZoneController::Configuration config;
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller;
};

TEST_P(ActiveModeTest, TargetTemperatureIsValid)
{
    EXPECT_NE(controller.targetTemperature(), std::nullopt);
}

TEST_P(ActiveModeTest, OverrideTemperatureOvershoot)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.overrideTargetTemperature(250);
    EXPECT_EQ(controller.targetTemperature(), 250);
    EXPECT_TRUE(controller.targetTemperatureOverrideActive());

    controller.inputTemperature(200);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(250);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(250 + config.heatingOvershoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, OverrideTemperatureUndershoot)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.overrideTargetTemperature(250);
    EXPECT_EQ(controller.targetTemperature(), 250);
    EXPECT_TRUE(controller.targetTemperatureOverrideActive());

    controller.inputTemperature(260);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(250);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(250 - config.heatingUndershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, OverrideTimesOut)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    const auto originalTargetTemperature = controller.targetTemperature();

    controller.overrideTargetTemperature(250);
    EXPECT_EQ(controller.targetTemperature(), 250);
    EXPECT_TRUE(controller.targetTemperatureOverrideActive());
    EXPECT_EQ(
        controller.targetTemperatureOverrideRemainingSeconds(),
        HeatingZoneController::Configuration{}.overrideTimeoutSeconds
    );

    controller.task(HeatingZoneController::Configuration{}.overrideTimeoutSeconds * 1000);
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_EQ(controller.targetTemperature(), originalTargetTemperature);
}

TEST_P(ActiveModeTest, OverrideCanBeReset)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    const auto originalTargetTemperature = controller.targetTemperature();

    controller.overrideTargetTemperature(250);
    EXPECT_EQ(controller.targetTemperature(), 250);
    EXPECT_TRUE(controller.targetTemperatureOverrideActive());

    controller.resetTargetTemperature();
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_EQ(controller.targetTemperature(), originalTargetTemperature);
}

TEST_P(ActiveModeTest, OverrideControlsHeatingWithoutTemperatureChange)
{
    controller.inputTemperature(240);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    EXPECT_FALSE(controller.callingForHeating());

    controller.overrideTargetTemperature(250);
    EXPECT_TRUE(controller.callingForHeating());

    controller.resetTargetTemperature();
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, KeepHeatingOnAfterBoostTimesOut)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(100);

    EXPECT_TRUE(controller.callingForHeating());

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());

    controller.task(HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000);
    EXPECT_FALSE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, KeepHeatingOnAfterBoostStoppedManually)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(100);

    EXPECT_TRUE(controller.callingForHeating());

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());

    controller.stopBoost();
    EXPECT_FALSE(controller.boostActive());
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingStartsAboveUndershootThresholdWhenFurnaceIsAlreadyRunning)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(controller.targetTemperature().value());
    EXPECT_FALSE(controller.callingForHeating());

    // Assuming undershoot is 5
    controller.inputTemperature(controller.targetTemperature().value() - 1);
    EXPECT_FALSE(controller.callingForHeating());

    controller.handleFurnaceHeatingChanged(true);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingDoesntStartWhenTheWindowIsOpen)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.setWindowOpened(true);
    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingStartsAfterOpenWindowLockoutDisengagedAndTemperatureIsLow)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.setWindowOpened(true);
    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.openWindowLockoutActive());
    EXPECT_EQ(controller.openWindowLockoutRemainingMs(), 0);

    controller.setWindowOpened(false);
    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_EQ(controller.openWindowLockoutRemainingMs(), 10 * 60 * 1000);

    controller.task(10 * 60 * 1000);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingDoesntStartAfterOpenWindowLockoutDisengagedAndTemperatureIsHigh)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.setWindowOpened(true);
    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.openWindowLockoutActive());
    EXPECT_EQ(controller.openWindowLockoutRemainingMs(), 0);

    controller.setWindowOpened(false);
    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_EQ(controller.openWindowLockoutRemainingMs(), 10 * 60 * 1000);

    controller.inputTemperature(240);
    controller.task(10 * 60 * 1000);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingStopsAfterOpeningWindowAndTemperatureIsLow)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.setWindowOpened(false);
    controller.inputTemperature(100);
    EXPECT_TRUE(controller.callingForHeating());

    controller.setWindowOpened(true);
    controller.inputTemperature(100);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingDoesntStartDuringBoostWhenTheWindowIsOpen)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(controller.targetTemperature().value());

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.callingForHeating());

    controller.setWindowOpened(true);
    EXPECT_FALSE(controller.callingForHeating());

    controller.setWindowOpened(false);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingStartsAfterClosingWindowDuringBoost)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(controller.targetTemperature().value());

    controller.setWindowOpened(true);
    controller.startOrExtendBoost();
    EXPECT_FALSE(controller.callingForHeating());

    controller.setWindowOpened(false);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST_P(ActiveModeTest, HeatingStopsAfterOpeningWindowDuringBoost)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(controller.targetTemperature().value());

    controller.setWindowOpened(false);
    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.callingForHeating());

    controller.setWindowOpened(true);
    EXPECT_FALSE(controller.callingForHeating());
}

INSTANTIATE_TEST_SUITE_P(
    HeatingZoneController,
    ActiveModeTest,
    ::testing::Values(
        HeatingZoneController::Mode::Auto,
        HeatingZoneController::Mode::Holiday
    )
);

#pragma endregion

#pragma region Test for all existing modes

class AllModesTest
    : public ::testing::TestWithParam<HeatingZoneController::Mode>
{
public:
    AllModesTest()
        : config{
            .heatingOvershoot = 50,
            .heatingUndershoot = 50,
            .holidayModeTemperature = 100
        }
        , controller{ config, schedule }
    {
        controller.setMode(GetParam());

        // Clear the state change indicator flag
        (void)controller.saveState();
    }

    HeatingZoneController::Configuration config;
    HeatingZoneController::Schedule schedule{{}};
    HeatingZoneController controller;
};

TEST_P(AllModesTest, ModeIsCorrect)
{
    EXPECT_EQ(controller.mode(), GetParam());
}

TEST_P(AllModesTest, BoostDefaultStopped)
{
    EXPECT_FALSE(controller.boostActive());
}

TEST_P(AllModesTest, BoostCanBeStarted)
{
    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_EQ(
        controller.boostRemainingSeconds(),
        HeatingZoneController::Configuration{}.boostInitialDurationSeconds
    );
}

TEST_P(AllModesTest, BoostCanBeExtended)
{
    controller.startOrExtendBoost();
    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.boostActive());
    EXPECT_EQ(
        controller.boostRemainingSeconds(),
        HeatingZoneController::Configuration{}.boostInitialDurationSeconds
        + HeatingZoneController::Configuration{}.boostExtensionDurationSeconds
    );
}

TEST_P(AllModesTest, BoostTimesOutAfterStarted)
{
    controller.startOrExtendBoost();
    controller.task(HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000);
    EXPECT_FALSE(controller.boostActive());
    EXPECT_EQ(controller.boostRemainingSeconds(), 0);
}

TEST_P(AllModesTest, BoostTimesOutAfterExtended)
{
    controller.startOrExtendBoost();
    controller.startOrExtendBoost();
    controller.task(HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000);
    EXPECT_TRUE(controller.boostActive());
    controller.task(HeatingZoneController::Configuration{}.boostExtensionDurationSeconds * 1000);
    EXPECT_FALSE(controller.boostActive());
    EXPECT_EQ(controller.boostRemainingSeconds(), 0);
}

TEST_P(AllModesTest, BoostCausesCallingForHeating)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(300);
    EXPECT_FALSE(controller.callingForHeating());

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.callingForHeating());

    controller.task(HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(AllModesTest, StoppingBoostEndsCallingForHeating)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(300);
    EXPECT_FALSE(controller.callingForHeating());

    controller.startOrExtendBoost();
    EXPECT_TRUE(controller.callingForHeating());

    controller.stopBoost();
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(AllModesTest, NoCallingForHeatingWithHighTemperature)
{
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(300);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST_P(AllModesTest, SettingModeChangesState)
{
    EXPECT_FALSE(controller.stateChanged());

    controller.setMode(HeatingZoneController::Mode::Auto);

    EXPECT_TRUE(controller.stateChanged());
    EXPECT_EQ(controller.saveState().mode, HeatingZoneController::Mode::Auto);
}

TEST_P(AllModesTest, SettingHighTargetChangesState)
{
    EXPECT_FALSE(controller.stateChanged());

    controller.setHighTargetTemperature(230);

    EXPECT_TRUE(controller.stateChanged());
    EXPECT_EQ(controller.saveState().highTargetTemperature, 230);
}

TEST_P(AllModesTest, SettingLowTargetChangesState)
{
    EXPECT_FALSE(controller.stateChanged());

    controller.setLowTargetTemperature(230);

    EXPECT_TRUE(controller.stateChanged());
    EXPECT_EQ(controller.saveState().lowTargetTemperature, 230);
}

TEST_P(AllModesTest, LoadingStateChangesMode)
{
    controller.setMode(HeatingZoneController::Mode::Off);

    controller.loadState(
        HeatingZoneController::State{
            .mode = HeatingZoneController::Mode::Auto
        }
    );

    EXPECT_EQ(controller.mode(), HeatingZoneController::Mode::Auto);
}

TEST_P(AllModesTest, LoadingStateChangesHighTargetTemperature)
{
    controller.setHighTargetTemperature(230);

    controller.loadState(
        HeatingZoneController::State{
            .highTargetTemperature = 240
        }
    );

    EXPECT_EQ(controller.highTargetTemperature(), 240);
}

TEST_P(AllModesTest, LoadingStateChangesLowTargetTemperature)
{
    controller.setLowTargetTemperature(230);

    controller.loadState(
        HeatingZoneController::State{
            .lowTargetTemperature = 240
        }
    );

    EXPECT_EQ(controller.lowTargetTemperature(), 240);
}

INSTANTIATE_TEST_SUITE_P(
    HeatingZoneController,
    AllModesTest,
    ::testing::Values(
        HeatingZoneController::Mode::Off,
        HeatingZoneController::Mode::Auto,
        HeatingZoneController::Mode::Holiday
    )
);

#pragma endregion

#pragma region Other tests

uint32_t delta(const uint32_t start, const uint32_t now)
{
    return now - start;
}

TEST(UnsignedDelta, WithoutOverflow)
{
    EXPECT_EQ(delta(1, 2), 1);
}

TEST(UnsignedDelta, WithOverflow)
{
    EXPECT_EQ(delta(0xFFFFFFFFu, 0), 1);
    EXPECT_EQ(delta(0xFFFFFFFFu, 1), 2);
    EXPECT_EQ(delta(0xFFFFFFFEu, 0), 2);
}

#pragma endregion

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {
    }

    return 0;
}