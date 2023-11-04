#include <gtest/gtest.h>

#include <HeatingZoneController.h>

#include <optional>

/*
    Test case naming:
        Mode<mode>_Schedule<schedule>_Boost<boost>_Heating<heating>_<details>

        mode := Off | Auto | Holiday
        schedule := AllLow | AllHigh
        boost := Stopped | Started | Extended
        heating := Idle | Calling
        details: Extra info if needed
*/

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

class HeatingZoneControllerParameterized
    : public ::testing::TestWithParam<InputParams>
{
public:
    HeatingZoneControllerParameterized()
        : controller{
            HeatingZoneController::Configuration{
                .holidayModeTemperature = GetParam().holidayTargetTemperature
            }
        }
    {
        controller.setMode(GetParam().mode);
        controller.setLowTargetTemperature(GetParam().lowTargetTemperature);
        controller.setHighTargetTemperature(GetParam().highTargetTemperature);
        controller.inputTemperature(GetParam().inputTemperature);

        switch (GetParam().boost) {
            case InputParams::Boost::Stopped:
                controller.stopBoost();
                break;
            case InputParams::Boost::Started:
                controller.startOrExtendBoost();
                break;
            case InputParams::Boost::Extended:
                controller.startOrExtendBoost();
                controller.startOrExtendBoost();
                break;
        }

        // Must be called after Boost operations
        if (GetParam().taskSystemClockMillis.has_value()) {
            controller.task(GetParam().taskSystemClockMillis.value());
        }
    }

    HeatingZoneController controller;
};

TEST_P(HeatingZoneControllerParameterized, CallingForHeating)
{
    EXPECT_EQ(controller.callingForHeating(), GetParam().expectedCallingForHeating);
}

TEST_P(HeatingZoneControllerParameterized, BoostActive)
{
    EXPECT_EQ(controller.boostActive(), GetParam().expectedBoostActive);
}

TEST_P(HeatingZoneControllerParameterized, TargetTemperature)
{
    if (GetParam().expectedTargetTemperature.has_value()) {
        EXPECT_EQ(controller.targetTemperature(), GetParam().expectedTargetTemperature.value());
    }
}

TEST_P(HeatingZoneControllerParameterized, TargetTemperatureOverride)
{
    if (controller.mode() == HeatingZoneController::Mode::Auto) {
        const auto original = controller.targetTemperature();

        controller.overrideTargetTemperature(300);
        EXPECT_EQ(controller.targetTemperature(), 300);

        controller.resetTargetTemperature();
        EXPECT_EQ(controller.targetTemperature(), original);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Something,
    HeatingZoneControllerParameterized,
    testing::Values(
        // "Off" mode tests
        InputParams{},
        InputParams{
            .inputTemperature = 0
        },
        InputParams{
            .inputTemperature = 100
        },
        InputParams{
            .inputTemperature = 180
        },
        InputParams{
            .inputTemperature = 210
        },
        InputParams{
            .inputTemperature = 230
        },
        InputParams{
            .inputTemperature = 250
        },
        InputParams{
            .boost = InputParams::Boost::Started,
            .expectedCallingForHeating = true,
            .expectedBoostActive = true
        },
        InputParams{
            .boost = InputParams::Boost::Started,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .taskSystemClockMillis = HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000
        },
        InputParams{
            .boost = InputParams::Boost::Extended,
            .expectedCallingForHeating = true,
            .expectedBoostActive = true
        },
        InputParams{
            .boost = InputParams::Boost::Extended,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .taskSystemClockMillis =
                (HeatingZoneController::Configuration{}.boostInitialDurationSeconds
                    + HeatingZoneController::Configuration{}.boostExtensionDurationSeconds)
                * 1000
        },
        // "Holiday" mode tests
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .inputTemperature = 0,
            .expectedCallingForHeating = true,
            .expectedBoostActive = false,
            .expectedTargetTemperature = 180
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .inputTemperature = 100,
            .expectedCallingForHeating = true,
            .expectedBoostActive = false,
            .expectedTargetTemperature = 180
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .inputTemperature = 180,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .expectedTargetTemperature = 180
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .inputTemperature = 210,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .expectedTargetTemperature = 180
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .inputTemperature = 230,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .expectedTargetTemperature = 180
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .inputTemperature = 250,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .expectedTargetTemperature = 180
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .boost = InputParams::Boost::Started,
            .expectedCallingForHeating = true,
            .expectedBoostActive = true
        },
        InputParams{
            .boost = InputParams::Boost::Started,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .taskSystemClockMillis = HeatingZoneController::Configuration{}.boostInitialDurationSeconds * 1000
        },
        InputParams{
            .mode = HeatingZoneController::Mode::Holiday,
            .boost = InputParams::Boost::Extended,
            .expectedCallingForHeating = true,
            .expectedBoostActive = true
        },
        InputParams{
            .boost = InputParams::Boost::Extended,
            .expectedCallingForHeating = false,
            .expectedBoostActive = false,
            .taskSystemClockMillis =
                (HeatingZoneController::Configuration{}.boostInitialDurationSeconds
                    + HeatingZoneController::Configuration{}.boostExtensionDurationSeconds)
                * 1000
        }
    )
);

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
    EXPECT_EQ(controller.targetTemperature(), HeatingZoneController::DeciDegrees{});
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
        HeatingZoneController::Configuration{}.boostInitialDurationSeconds
        + HeatingZoneController::Configuration{}.boostExtensionDurationSeconds
    );

    controller.task(controller.boostRemainingSeconds() * 1000);
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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {
    }

    return 0;
}