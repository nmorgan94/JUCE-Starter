#include <juce_audio_basics/juce_audio_basics.h>
#include <samplerealm/BuildCommit.h>
#include <samplerealm/ParameterValues.h>
#include <samplerealm/PeakMeter.h>
#include <samplerealm/UiInfoJson.h>
#include <samplerealm/WasmExport.h>
#include "shared/ParameterSpecs.h"
#include "shared/EffectSettings.h"
#include "dsp/Effect.h"

//==============================================================================
namespace
{
    constexpr int maxBlockSize = 128;
    constexpr int maxChannels = PeakMeter::maxChannels;

    ParameterValues values { ParameterSpecs::all };

    Effect effect;
    PeakMeter meter { maxChannels };
    juce::AudioBuffer<float> buffer { maxChannels, maxBlockSize };
    int numChannels = maxChannels;
    double currentSampleRate = 48000.0;
    Transport transport;
}

WASM_EXPORT (uiInfo) const char* uiInfo()
{
    static const auto json = uiInfoJson (ParameterSpecs::all, PLUGIN_VERSION, buildCommit);
    return json.c_str();
}

// As isBusesLayoutSupported: mono or stereo, the same in and out
WASM_EXPORT (prepare) void prepare (double sampleRate, int channels)
{
    numChannels = std::clamp (channels, 1, maxChannels);
    currentSampleRate = sampleRate;
    meter.setNumChannels (numChannels);
    buffer.clear();
    effect.prepare (sampleRate);
}

WASM_EXPORT (setParam) void setParam (int index, float value)
{
    values.set (index, value);
}

// The page plays the host: render moves the position on while playing
WASM_EXPORT (setTransport) void setTransport (double bpm, bool playing, double ppq)
{
    transport = { bpm, playing, ppq };
}

// Write the input here, and read the output back after render
WASM_EXPORT (channel) float* channel (int index)
{
    return buffer.getWritePointer (std::clamp (index, 0, maxChannels - 1));
}

WASM_EXPORT (render) void render (int numSamples)
{
    juce::AudioBuffer<float> block { buffer.getArrayOfWritePointers(), numChannels, std::clamp (numSamples, 0, maxBlockSize) };

    effect.setSettings (makeEffectSettings (values));
    effect.process (block, transport);
    meter.process (block);
    transport.advance (block.getNumSamples(), currentSampleRate);
}

// The loudest output sample since the last call, as linear gain
WASM_EXPORT (peak) float peak (int index)
{
    return index >= 0 && index < numChannels ? meter.readAndReset (index) : 0.0f;
}
