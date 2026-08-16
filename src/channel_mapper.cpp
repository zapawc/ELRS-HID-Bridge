#include "channel_mapper.h"


ChannelMapper::ChannelMapper(
    const BridgeConfiguration& configuration
)
    : configuration(configuration)
{
}


uint16_t ChannelMapper::mappedValue(
    const NormalizedChannels& channels,
    const AxisMapping& mapping
) const
{
    const uint16_t value =
        channels.get(
            mapping.channel
        );


    if (!mapping.inverted)
    {
        return value;
    }


    return static_cast<uint16_t>(
        NormalizedChannels::MAX -
        value
    );
}


ChannelMapper::SwitchPosition
ChannelMapper::decodeThreePosition(
    uint16_t value
) const
{
    if (
        value <
        configuration.switchLowThreshold
    )
    {
        return SwitchPosition::Low;
    }


    if (
        value >
        configuration.switchHighThreshold
    )
    {
        return SwitchPosition::High;
    }


    return SwitchPosition::Center;
}


bool ChannelMapper::decodeTwoPosition(
    uint16_t value
) const
{
    return
        value >
        configuration.switchHighThreshold;
}


void ChannelMapper::setButton(
    ChannelState& state,
    uint8_t buttonNumber
)
{
    if (
        buttonNumber < 1 ||
        buttonNumber > 32
    )
    {
        return;
    }


    state.buttons |=
        (1UL << (buttonNumber - 1));
}


void ChannelMapper::update(
    const NormalizedChannels& channels,
    ChannelState& state
) const
{
    // -------------------------------------------------------------------------
    // Primary proportional controls
    // -------------------------------------------------------------------------

    state.roll =
        mappedValue(
            channels,
            configuration.roll
        );


    state.pitch =
        mappedValue(
            channels,
            configuration.pitch
        );


    state.throttle =
        mappedValue(
            channels,
            configuration.throttle
        );


    state.yaw =
        mappedValue(
            channels,
            configuration.yaw
        );


    // -------------------------------------------------------------------------
    // Rebuild button state on every update.
    // -------------------------------------------------------------------------

    state.buttons = 0;


    // -------------------------------------------------------------------------
    // Two-position switches
    // -------------------------------------------------------------------------

    for (
        uint8_t index = 0;
        index <
            BridgeConfiguration::
                TWO_POSITION_SWITCH_COUNT;
        ++index
    )
    {
        const TwoPositionSwitchMapping& mapping =
            configuration
                .twoPositionSwitches[index];


        if (
            decodeTwoPosition(
                channels.get(
                    mapping.channel
                )
            )
        )
        {
            setButton(
                state,
                mapping.button
            );
        }
    }


    // -------------------------------------------------------------------------
    // Three-position switches
    //
    // Low    -> no button
    // Center -> centerButton
    // High   -> highButton
    // -------------------------------------------------------------------------

    for (
        uint8_t index = 0;
        index <
            BridgeConfiguration::
                THREE_POSITION_SWITCH_COUNT;
        ++index
    )
    {
        const ThreePositionSwitchMapping& mapping =
            configuration
                .threePositionSwitches[index];


        switch (
            decodeThreePosition(
                channels.get(
                    mapping.channel
                )
            )
        )
        {
            case SwitchPosition::Center:
            {
                setButton(
                    state,
                    mapping.centerButton
                );

                break;
            }


            case SwitchPosition::High:
            {
                setButton(
                    state,
                    mapping.highButton
                );

                break;
            }


            case SwitchPosition::Low:
            {
                break;
            }
        }
    }


    // -------------------------------------------------------------------------
    // Additional proportional controls
    // -------------------------------------------------------------------------

    state.auxAnalog1 =
        mappedValue(
            channels,
            configuration.auxAnalog1
        );


    state.auxAnalog2 =
        mappedValue(
            channels,
            configuration.auxAnalog2
        );


    state.auxAnalog3 =
        mappedValue(
            channels,
            configuration.auxAnalog3
        );


    state.auxAnalog4 =
        mappedValue(
            channels,
            configuration.auxAnalog4
        );
}