#pragma once

class CrsfSelfTest
{
public:
    // Runs deterministic regression tests of the CRSF receive path.
    //
    // Returns true only if all tests pass.
    static bool run();
};
