#include "Effect.h"

//==============================================================================
void Effect::prepare (double sampleRate)
{
    juce::ignoreUnused (sampleRate);
}

void Effect::setSettings (const Settings& newSettings)
{
    settings = newSettings;
}

void Effect::process (juce::AudioBuffer<float>& buffer, const Transport& transport)
{
    juce::ignoreUnused (transport);

    const float gainLinear = juce::Decibels::decibelsToGain (settings.gainDb, -60.0f);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        juce::FloatVectorOperations::multiply (buffer.getWritePointer (channel), gainLinear, buffer.getNumSamples());
}
