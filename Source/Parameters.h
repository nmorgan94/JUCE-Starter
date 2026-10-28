#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "shared/ParameterSpecs.h"

//==============================================================================
namespace Parameters
{
    // Bump this when adding or restructuring parameters in a new plugin version.
    // All ParameterIDs must use this so there is one place to update.
    constexpr int versionHint = 1;

    // Parameter IDs — use these constants everywhere instead of raw strings.
    // inline const means one definition shared across all translation units (C++17).
    inline const juce::ParameterID masterGainId { ParameterSpecs::masterGain.id, versionHint };

    // Built from ParameterSpecs::all, in its order
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
}
