#include "Parameters.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createLayout()
{
    return {
        std::make_unique<juce::AudioParameterFloat>(
            masterGainId,
            "Master Gain",
            juce::NormalisableRange<float> (-60.0f, 6.0f, 0.1f, 1.0f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("dB"))
    };
}
