#include "failsafe_policy_self_test.h"

#include "channel_state.h"
#include "failsafe_policy.h"
#include "normalized_channels.h"


namespace
{
    bool runDefinedOutputTest()
    {
        // Start every field away from its intended failsafe value so the test
        // proves FailsafePolicy actively defines the complete HID state.
        ChannelState state;

        state.roll = 1000;
        state.pitch = 2000;
        state.throttle = 3000;
        state.yaw = 4000;

        state.auxAnalog1 = 5000;
        state.auxAnalog2 = 6000;
        state.auxAnalog3 = 7000;
        state.auxAnalog4 = 8000;

        state.buttons = 0xFFFFFFFFu;


        FailsafePolicy policy;

        policy.apply(
            state
        );


        if (
            state.roll !=
            NormalizedChannels::MID
        )
        {
            return false;
        }

        if (
            state.pitch !=
            NormalizedChannels::MID
        )
        {
            return false;
        }

        if (
            state.throttle !=
            NormalizedChannels::MIN
        )
        {
            return false;
        }

        if (
            state.yaw !=
            NormalizedChannels::MID
        )
        {
            return false;
        }


        if (
            state.auxAnalog1 !=
            NormalizedChannels::MID
        )
        {
            return false;
        }

        if (
            state.auxAnalog2 !=
            NormalizedChannels::MID
        )
        {
            return false;
        }

        if (
            state.auxAnalog3 !=
            NormalizedChannels::MID
        )
        {
            return false;
        }

        if (
            state.auxAnalog4 !=
            NormalizedChannels::MID
        )
        {
            return false;
        }


        if (state.buttons != 0)
        {
            return false;
        }


        return true;
    }


    bool runIdempotenceTest()
    {
        ChannelState state;

        FailsafePolicy policy;


        policy.apply(
            state
        );


        const ChannelState first =
            state;


        policy.apply(
            state
        );


        return
            state.roll == first.roll &&
            state.pitch == first.pitch &&
            state.throttle == first.throttle &&
            state.yaw == first.yaw &&
            state.auxAnalog1 == first.auxAnalog1 &&
            state.auxAnalog2 == first.auxAnalog2 &&
            state.auxAnalog3 == first.auxAnalog3 &&
            state.auxAnalog4 == first.auxAnalog4 &&
            state.buttons == first.buttons;
    }
}


bool FailsafePolicySelfTest::run()
{
    return
        runDefinedOutputTest() &&
        runIdempotenceTest();
}
