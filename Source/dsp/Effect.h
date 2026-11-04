#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../shared/Transport.h"

//==============================================================================
class Effect
{
public:
    struct Settings
    {
        float gainDb = 0.0f;
    };

    void prepare (double sampleRate);
    void setSettings (const Settings& newSettings);

    void process (juce::AudioBuffer<float>& buffer, const Transport& transport);

private:
    Settings settings;
};
