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

#pragma region General tests

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
    controller.setLowTargetTemperature(210);

    controller.inputTemperature(230);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(210);
    EXPECT_FALSE(controller.callingForHeating());

    controller.inputTemperature(210 - undershoot);
    EXPECT_TRUE(controller.callingForHeating());
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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {
    }

    return 0;
}