#include <gtest/gtest.h>

#include <HeatingZoneController.h>

namespace TestUtils
{
    HeatingZoneController::ScheduleData generateFullSchedule()
    {
        HeatingZoneController::ScheduleData data{{}};
        for (auto& byte : data) {
            byte = 0xFF;
        }
        return data;
    }
}

TEST(HeatingZoneController, InitialState)
{
    HeatingZoneController fc{
        HeatingZoneController::Configuration{
        }
    };

    EXPECT_FALSE(fc.boostActive());
    EXPECT_FALSE(fc.callingForHeating());
    EXPECT_EQ(fc.mode(), HeatingZoneController::Mode::Off);
    EXPECT_FALSE(fc.targetTemperatureOverrideActive());
    EXPECT_EQ(fc.targetTemperature(), HeatingZoneController::DeciDegrees{});
}

TEST(HeatingZoneController, LowTargetTempWithEmptySchedule)
{
    HeatingZoneController fc{
        HeatingZoneController::Configuration{
        }
    };

    fc.setHighTargetTemperature(230);
    fc.setLowTargetTemperature(210);

    EXPECT_EQ(fc.targetTemperature(), 210);
}

TEST(HeatingZoneController, HighTargetTempWithFullSchedule)
{
    HeatingZoneController fc{
        HeatingZoneController::Configuration{
            .scheduleData = TestUtils::generateFullSchedule()
        }
    };

    fc.setHighTargetTemperature(230);
    fc.setLowTargetTemperature(210);

    EXPECT_EQ(fc.targetTemperature(), 230);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {
    }

    return 0;
}