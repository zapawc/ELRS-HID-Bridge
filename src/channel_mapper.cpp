#include "channel_mapper.h"

namespace
{
    constexpr uint16_t SWITCH_LOW_THRESHOLD = 16384;
    constexpr uint16_t SWITCH_HIGH_THRESHOLD = 49151;
}


ChannelMapper::SwitchPosition
ChannelMapper::decodeThreePosition(uint16_t value)
{
    if (value < SWITCH_LOW_THRESHOLD)
    {
        return SwitchPosition::Low;
    }

    if (value > SWITCH_HIGH_THRESHOLD)
    {
        return SwitchPosition::High;
    }

    return SwitchPosition::Center;
}


bool ChannelMapper::decodeTwoPosition(uint16_t value)
{
    return value > SWITCH_HIGH_THRESHOLD;
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
    // Primary controls
    //
    // CH1 -> Roll
    // CH2 -> Pitch
    // CH3 -> Throttle
    // CH4 -> Yaw
    //
    // Confirmed HID orientation:
    //
    // Roll     = normal
    // Pitch    = inverted
    // Throttle = normal
    // Yaw      = normal
    // -------------------------------------------------------------------------

    state.roll =
        channels.get(ChannelIndex::CH1);

    state.pitch =
        NormalizedChannels::MAX -
        channels.get(ChannelIndex::CH2);

    state.throttle =
        channels.get(ChannelIndex::CH3);

    state.yaw =
        channels.get(ChannelIndex::CH4);


    // -------------------------------------------------------------------------
    // AUX switch controls
    // -------------------------------------------------------------------------

    state.buttons = 0;


    // SF / CH5
    // Up   -> released
    // Down -> Button 1

    if (
        decodeTwoPosition(
            channels.get(ChannelIndex::CH5)
        )
    )
    {
        setButton(state, 1);
    }


    // SA / CH6
    // Up     -> no button
    // Middle -> Button 2
    // Down   -> Button 3

    switch (
        decodeThreePosition(
            channels.get(ChannelIndex::CH6)
        )
    )
    {
        case SwitchPosition::Center:
            setButton(state, 2);
            break;

        case SwitchPosition::High:
            setButton(state, 3);
            break;

        case SwitchPosition::Low:
            break;
    }


    // SB / CH7
    // Up     -> no button
    // Middle -> Button 4
    // Down   -> Button 5

    switch (
        decodeThreePosition(
            channels.get(ChannelIndex::CH7)
        )
    )
    {
        case SwitchPosition::Center:
            setButton(state, 4);
            break;

        case SwitchPosition::High:
            setButton(state, 5);
            break;

        case SwitchPosition::Low:
            break;
    }


    // SC / CH8
    // Up     -> no button
    // Middle -> Button 6
    // Down   -> Button 7

    switch (
        decodeThreePosition(
            channels.get(ChannelIndex::CH8)
        )
    )
    {
        case SwitchPosition::Center:
            setButton(state, 6);
            break;

        case SwitchPosition::High:
            setButton(state, 7);
            break;

        case SwitchPosition::Low:
            break;
    }


    // SD / CH9
    // Up     -> no button
    // Middle -> Button 8
    // Down   -> Button 9

    switch (
        decodeThreePosition(
            channels.get(ChannelIndex::CH9)
        )
    )
    {
        case SwitchPosition::Center:
            setButton(state, 8);
            break;

        case SwitchPosition::High:
            setButton(state, 9);
            break;

        case SwitchPosition::Low:
            break;
    }


    // SE / CH10
    // Up     -> no button
    // Middle -> Button 10
    // Down   -> Button 11

    switch (
        decodeThreePosition(
            channels.get(ChannelIndex::CH10)
        )
    )
    {
        case SwitchPosition::Center:
            setButton(state, 10);
            break;

        case SwitchPosition::High:
            setButton(state, 11);
            break;

        case SwitchPosition::Low:
            break;
    }


    // SG / CH11
    // Up     -> no button
    // Middle -> Button 12
    // Down   -> Button 13

    switch (
        decodeThreePosition(
            channels.get(ChannelIndex::CH11)
        )
    )
    {
        case SwitchPosition::Center:
            setButton(state, 12);
            break;

        case SwitchPosition::High:
            setButton(state, 13);
            break;

        case SwitchPosition::Low:
            break;
    }


    // SH / CH12
    // Released -> no button
    // Pressed  -> Button 14

    if (
        decodeTwoPosition(
            channels.get(ChannelIndex::CH12)
        )
    )
    {
        setButton(state, 14);
    }


    // -------------------------------------------------------------------------
    // Additional proportional controls
    //
    // CH13 -> Z
    // CH14 -> X Rotation
    // CH15 -> Y Rotation
    // CH16 -> Z Rotation
    // -------------------------------------------------------------------------

    state.auxAnalog1 =
        channels.get(ChannelIndex::CH13);

    state.auxAnalog2 =
        channels.get(ChannelIndex::CH14);

    state.auxAnalog3 =
        channels.get(ChannelIndex::CH15);

    state.auxAnalog4 =
        channels.get(ChannelIndex::CH16);
}