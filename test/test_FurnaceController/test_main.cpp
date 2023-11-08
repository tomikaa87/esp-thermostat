#include <gtest/gtest.h>

#include <HeatingZoneController.h>

#include <optional>

#pragma region Utilities

namespace TestUtils
{
    HeatingZoneController::ScheduleData generateAllHighSchedule()
    {
        HeatingZoneController::ScheduleData data{{}};
        for (auto& byte : data) {
            byte = 0xFF;
        }
        return data;
    }

    HeatingZoneController::ScheduleData generateHighScheduleForEveryFirstHalfHour()
    {
        HeatingZoneController::ScheduleData data{{}};
        for (auto& byte : data) {
            byte = 0b01010101;
        }
        return data;
    }

    HeatingZoneController::ScheduleData generateHighScheduleForEverySecondHalfHour()
    {
        HeatingZoneController::ScheduleData data{{}};
        for (auto& byte : data) {
            byte = 0b10101010;
        }
        return data;
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

    HeatingZoneController::ScheduleData scheduleData{};
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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
        }
    };

    EXPECT_FALSE(controller.boostActive());
    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_EQ(controller.mode(), HeatingZoneController::Mode::Off);
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_FALSE(controller.targetTemperature().has_value());
}

TEST(HeatingZoneController, ModeAuto_ScheduleAllLow_BoostStopped_HeatingIdle_LowTargetTemp)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
        }
    };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    EXPECT_EQ(controller.targetTemperature(), 210);
}

TEST(HeatingZoneController, ModeAuto_ScheduleAllHigh_BoostStopped_HeatingIdle_HighTargetTemp)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    EXPECT_EQ(controller.targetTemperature(), 230);
}

TEST(HeatingZoneController, ModeOff_ScheduleAllLow_BoostStopped_HeatingIdle)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
        }
    };

    controller.setMode(HeatingZoneController::Mode::Off);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(220);

    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllHigh_BoostStopped_HeatingIdle)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

    controller.setMode(HeatingZoneController::Mode::Off);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.inputTemperature(200);

    EXPECT_FALSE(controller.callingForHeating());
    EXPECT_FALSE(controller.boostActive());
}

TEST(HeatingZoneController, ModeOff_ScheduleAllHigh_BoostStarted_HeatingCalling)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .holidayModeTemperature = 150
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .holidayModeTemperature = 150,
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    controller.setMode(HeatingZoneController::Mode::Holiday);

    EXPECT_EQ(controller.targetTemperature(), 150);
}

TEST(HeatingZoneController, ModeHoliday_ScheduleAllLow_BoostStarted_HeatingCalling)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .holidayModeTemperature = 150
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .holidayModeTemperature = 150
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

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
    const HeatingZoneController::DeciDegrees overshoot{ 5 };

    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .heatingOvershoot = overshoot,
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(200);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(230);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(230 + overshoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, HighTargetTemperatureUndershoot)
{
    const HeatingZoneController::DeciDegrees undershoot{ 5 };

    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .heatingUndershoot = undershoot,
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);

    controller.inputTemperature(250);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(230);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(230 - undershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, LowTargetTemperatureChangeControlsHeating)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
        }
    };

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
    const HeatingZoneController::DeciDegrees overshoot{ 5 };

    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .heatingOvershoot = overshoot
        }
    };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    EXPECT_EQ(controller.targetTemperature(), 210);

    controller.inputTemperature(200);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(210);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(210 + overshoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, LowTargetTemperatureUndershoot)
{
    const HeatingZoneController::DeciDegrees undershoot{ 5 };

    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .heatingUndershoot = undershoot
        }
    };

    controller.setMode(HeatingZoneController::Mode::Auto);
    controller.setHighTargetTemperature(230);
    controller.setLowTargetTemperature(210);
    EXPECT_EQ(controller.targetTemperature(), 210);

    controller.inputTemperature(230);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(210);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(210 - undershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, HolidayTargetTemperatureOvershoot)
{
    const HeatingZoneController::DeciDegrees overshoot{ 5 };
    const HeatingZoneController::DeciDegrees target{ 180 };

    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .heatingOvershoot = overshoot,
            .holidayModeTemperature = target
        }
    };

    controller.setMode(HeatingZoneController::Mode::Holiday);
    EXPECT_EQ(controller.targetTemperature(), target);

    controller.inputTemperature(target - 20);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(target);
    EXPECT_TRUE(controller.callingForHeating());

    controller.inputTemperature(target + overshoot);
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, HolidayTargetTemperatureUndershoot)
{
    const HeatingZoneController::DeciDegrees undershoot{ 5 };
    const HeatingZoneController::DeciDegrees target{ 180 };

    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .heatingUndershoot = undershoot,
            .holidayModeTemperature = target
        }
    };

    controller.setMode(HeatingZoneController::Mode::Holiday);
    EXPECT_EQ(controller.targetTemperature(), target);

    controller.inputTemperature(target + 20);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(target);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(target - undershoot);
    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, OffModeReportsNoTargetTemperature)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

    controller.setMode(HeatingZoneController::Mode::Off);

    EXPECT_EQ(controller.targetTemperature(), std::nullopt);
}

TEST(HeatingZoneController, OffModeDoesNotCallForHeatingAutomatically)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.overrideTargetTemperature(250);
    controller.inputTemperature(100);

    EXPECT_EQ(controller.targetTemperatureOverrideRemainingSeconds(), 0);
    EXPECT_FALSE(controller.targetTemperatureOverrideActive());
    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToOffModeDisablesOverride)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

    controller.setMode(HeatingZoneController::Mode::Auto);

    controller.setLowTargetTemperature(210);
    controller.inputTemperature(100);

    EXPECT_TRUE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Off);

    EXPECT_FALSE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToAutoModeTurnsOnHeating)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.setLowTargetTemperature(210);
    controller.inputTemperature(100);

    EXPECT_FALSE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Auto);

    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, SwitchingToAutoModeTurnsOnHeatingWithHighTarget)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{}
    };

    controller.setMode(HeatingZoneController::Mode::Off);

    controller.inputTemperature(100);

    EXPECT_FALSE(controller.callingForHeating());

    controller.setMode(HeatingZoneController::Mode::Holiday);

    EXPECT_TRUE(controller.callingForHeating());
}

TEST(HeatingZoneController, AlwaysHighTargetTemperatureForAllHighSchedule)
{
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateAllHighSchedule()
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = {}
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateHighScheduleForEveryFirstHalfHour()
        }
    };

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
    HeatingZoneController controller{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateHighScheduleForEverySecondHalfHour()
        }
    };

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
        : controller{
            HeatingZoneController::Configuration{
                .heatingOvershoot = overshoot,
                .heatingUndershoot = undershoot,
                .holidayModeTemperature = 180
            }
        }
    {
        controller.setMode(GetParam());
    }

    HeatingZoneController::DeciDegrees overshoot{ 5 };
    HeatingZoneController::DeciDegrees undershoot{ 5 };
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

    controller.inputTemperature(250 + overshoot);
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

    controller.inputTemperature(250 - undershoot);
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
        : controller{
            HeatingZoneController::Configuration{
                .heatingOvershoot = 50,
                .heatingUndershoot = 50,
                .holidayModeTemperature = 180
            }
        }
    {
        controller.setMode(GetParam());
    }

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