#include "bridge_configuration.h"


BridgeConfiguration
BridgeConfiguration::defaults()
{
    BridgeConfiguration config;

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
    // Roll     normal
    // Pitch    inverted
    // Throttle normal
    // Yaw      normal
    // -------------------------------------------------------------------------

    config.roll.channel =
        ChannelIndex::CH1;
    config.roll.inverted =
        false;


    config.pitch.channel =
        ChannelIndex::CH2;

    config.pitch.inverted =
        true;


    config.throttle.channel =
        ChannelIndex::CH3;
    config.throttle.inverted =
        false;


    config.yaw.channel =
        ChannelIndex::CH4;

    config.yaw.inverted =
        false;


    // -------------------------------------------------------------------------
    // Additional proportional controls
    //
    // CH13 -> HID Z
    // CH14 -> HID Rx
    // CH15 -> HID Ry
    // CH16 -> HID Rz
    // -------------------------------------------------------------------------

    config.auxAnalog1.channel =
        ChannelIndex::CH13;

    config.auxAnalog1.inverted =
        false;


    config.auxAnalog2.channel =
        ChannelIndex::CH14;

    config.auxAnalog2.inverted =
        false;


    config.auxAnalog3.channel =
        ChannelIndex::CH15;

    config.auxAnalog3.inverted =
        false;


    config.auxAnalog4.channel =
        ChannelIndex::CH16;

    config.auxAnalog4.inverted =
        false;


    // -------------------------------------------------------------------------
    // Two-position switches
    //
    // CH5 / SF
    //   High -> Button 1
    //
    // CH12 / SH
    //   High -> Button 14
    // -------------------------------------------------------------------------

    config.twoPositionSwitches[0].channel =
        ChannelIndex::CH5;

    config.twoPositionSwitches[0].button =
        1;


    config.twoPositionSwitches[1].channel =
        ChannelIndex::CH12;

    config.twoPositionSwitches[1].button =
        14;


    // -------------------------------------------------------------------------
    // Three-position switches
    //
    // Low    -> no button
    // Center -> first button
    // High   -> second button
    // -------------------------------------------------------------------------


    // CH6 / SA
    //
    // Center -> Button 2
    // High   -> Button 3

    config.threePositionSwitches[0].channel =
        ChannelIndex::CH6;

    config.threePositionSwitches[0].centerButton =
        2;

    config.threePositionSwitches[0].highButton =
        3;


    // CH7 / SB
    //
    // Center -> Button 4
    // High   -> Button 5

    config.threePositionSwitches[1].channel =
        ChannelIndex::CH7;

    config.threePositionSwitches[1].centerButton =
        4;

    config.threePositionSwitches[1].highButton =
        5;


    // CH8 / SC
    //
    // Center -> Button 6
    // High   -> Button 7

    config.threePositionSwitches[2].channel =
        ChannelIndex::CH8;

    config.threePositionSwitches[2].centerButton =
        6;

    config.threePositionSwitches[2].highButton =
        7;


    // CH9 / SD
    //
    // Center -> Button 8
    // High   -> Button 9

    config.threePositionSwitches[3].channel =
        ChannelIndex::CH9;

    config.threePositionSwitches[3].centerButton =
        8;

    config.threePositionSwitches[3].highButton =
        9;


    // CH10 / SE
    //
    // Center -> Button 10
    // High   -> Button 11

    config.threePositionSwitches[4].channel =
        ChannelIndex::CH10;

    config.threePositionSwitches[4].centerButton =
        10;

    config.threePositionSwitches[4].highButton =
        11;


    // CH11 / SG
    //
    // Center -> Button 12
    // High   -> Button 13

    config.threePositionSwitches[5].channel =
        ChannelIndex::CH11;

    config.threePositionSwitches[5].centerButton =
        12;

    config.threePositionSwitches[5].highButton =
        13;


    // -------------------------------------------------------------------------
    // Switch thresholds
    // -------------------------------------------------------------------------

    config.switchLowThreshold =
        16384;

    config.switchHighThreshold =
        49151;


    // -------------------------------------------------------------------------
    // Receiver timeout
    // -------------------------------------------------------------------------

    config.receiverTimeoutMs =
        500;


    // -------------------------------------------------------------------------
    // Status LED
    //
    // The pre-parameter implementation used NeoPixel brightness 24/255
    // (~9.4%). Ten percent preserves the established visual level closely
    // while presenting a clean 0-100% user-facing setting.
    // -------------------------------------------------------------------------

    config.ledBrightnessPercent =
        10;


    return config;
}
