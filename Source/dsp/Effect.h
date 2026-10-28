#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

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

    void process (juce::AudioBuffer<float>& buffer);

private:
    Settings settings;
};
