#pragma once

#include <samplerealm/ParameterSpec.h>

//==============================================================================
namespace ParameterSpecs
{
    inline constexpr Spec masterGain { "masterGain", "Master Gain", Kind::real, -60.0f, 6.0f, 0.0f, 0.1f, 1.0f, "dB" };

    // The host's parameter order
    inline constexpr std::array all { &masterGain };

    static_assert (areValid (all));
}
