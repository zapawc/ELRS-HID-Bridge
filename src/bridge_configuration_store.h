#pragma once

#include "bridge_configuration.h"


class BridgeConfigurationStore
{
public:
    // Load a persisted configuration record from Arduino-Pico's emulated
    // EEPROM flash sector.
    //
    // Returns true only when the complete record passes
    // BridgeConfigurationRecord validation.
    //
    // On false, configuration is left unchanged so the caller safely retains
    // BridgeConfiguration::defaults().
    bool load(
        BridgeConfiguration& configuration
    ) const;


    // Serialize and persist the currently supported user settings using the
    // Arduino-Pico EEPROM emulation layer.
    //
    // Returns false if the configuration cannot be encoded or the flash commit
    // fails.
    bool save(
        const BridgeConfiguration& configuration
    ) const;
};
