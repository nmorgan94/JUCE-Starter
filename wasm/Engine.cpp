#include <juce_audio_basics/juce_audio_basics.h>
#include "shared/ParameterSpecs.h"
#include "shared/EffectSettings.h"
#include "shared/UiInfoJson.h"
#include "dsp/Effect.h"
#include "dsp/PeakMeter.h"

//==============================================================================
namespace
{
    constexpr int maxBlockSize = 128;
    constexpr int maxChannels = PeakMeter::maxChannels;

    constexpr auto numParams = static_cast<int> (ParameterSpecs::all.size());

    constexpr auto indexOf (const ParameterSpecs::Spec& spec)
    {
        return std::ranges::find (ParameterSpecs::all, &spec) - ParameterSpecs::all.begin();
    }

    auto defaultValues()
    {
        std::array<float, ParameterSpecs::all.size()> values;

        for (size_t i = 0; i < values.size(); ++i)
            values[i] = ParameterSpecs::all[i]->defaultValue;

        return values;
    }

    auto values = defaultValues();

    float valueOf (const ParameterSpecs::Spec& spec)
    {
        return values[static_cast<size_t> (indexOf (spec))];
    }

    Effect effect;
    PeakMeter meter { maxChannels };
    juce::AudioBuffer<float> buffer { maxChannels, maxBlockSize };
    int numChannels = maxChannels;
}

#define EXPORT(name) extern "C" __attribute__ ((export_name (#name)))

EXPORT (uiInfo) const char* uiInfo()
{
    static const auto json = uiInfoJson (PLUGIN_VERSION);
    return json.c_str();
}

// As isBusesLayoutSupported: mono or stereo, the same in and out
EXPORT (prepare) void prepare (double sampleRate, int channels)
{
    numChannels = std::clamp (channels, 1, maxChannels);
    meter.setNumChannels (numChannels);
    buffer.clear();
    effect.prepare (sampleRate);
}

EXPORT (setParam) void setParam (int index, float value)
{
    if (index >= 0 && index < numParams)
        values[static_cast<size_t> (index)] = value;
}

// Write the input here, and read the output back after render
EXPORT (channel) float* channel (int index)
{
    return buffer.getWritePointer (std::clamp (index, 0, maxChannels - 1));
}

EXPORT (render) void render (int numSamples)
{
    juce::AudioBuffer<float> block { buffer.getArrayOfWritePointers(), numChannels, std::clamp (numSamples, 0, maxBlockSize) };

    effect.setSettings (makeEffectSettings (valueOf));
    effect.process (block);
    meter.process (block);
}

// The loudest output sample since the last call, as linear gain
EXPORT (peak) float peak (int index)
{
    return index >= 0 && index < numChannels ? meter.readAndReset (index) : 0.0f;
}
