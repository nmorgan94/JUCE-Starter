#pragma once

#include "ParameterSpecs.h"
#include "../dsp/Effect.h"

//==============================================================================
template <typename ValueOf>
Effect::Settings makeEffectSettings (ValueOf&& valueOf)
{
    using namespace ParameterSpecs;

    Effect::Settings settings;
    settings.gainDb = valueOf (masterGain);
    return settings;
}
