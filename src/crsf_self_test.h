#pragma once

class CrsfSelfTest
{
public:
    // Runs deterministic tests of the CRSF RC-channel decode path.
    //
    // Returns true only if all tests pass.
    static bool run();
};