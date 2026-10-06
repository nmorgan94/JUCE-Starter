#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Master gain parameter has the expected range and default", "[parameters]")
{
    auto* parameter = dynamic_cast<juce::AudioParameterFloat*> (
        processor.getAPVTS().getParameter (Parameters::masterGainId.getParamID()));

    REQUIRE (parameter != nullptr);
    CHECK (parameter->getName (100) == "Master Gain");
    CHECK (parameter->getLabel() == "dB");
    CHECK_THAT (parameter->range.start, WithinAbs (-60.0, 1.0e-6));
    CHECK_THAT (parameter->range.end, WithinAbs (6.0, 1.0e-6));
    CHECK_THAT (parameter->get(), WithinAbs (0.0, 1.0e-6));
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Master gain scales the signal", "[dsp][gain]")
{
    const auto [gainDb, expectedGain] = GENERATE (table<float, float> ({
        {   0.0f, 1.0f },
        {  -6.0f, 0.501187f },
        {   6.0f, 1.995262f },
        { -60.0f, 0.0f },       // the bottom of the range is treated as silence
    }));

    CAPTURE (gainDb);
    setParameter (processor, Parameters::masterGainId, gainDb);

    constexpr float input = 0.5f;
    auto buffer = makeConstantBuffer (2, blockSize, input);
    process (processor, buffer);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        CHECK_THAT (buffer.getSample (channel, 0), WithinAbs (input * expectedGain, 1.0e-4));
        CHECK_THAT (buffer.getSample (channel, blockSize - 1), WithinAbs (input * expectedGain, 1.0e-4));
    }
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Output stays finite for any block size, input and gain", "[dsp]")
{
    // Includes a block larger than the size given to prepareToPlay, which some hosts send
    const auto numSamples = GENERATE (1, 64, blockSize, blockSize * 4);

    // Silence, denormals, full scale and very loud
    const auto amplitude = GENERATE (0.0f, 1.0e-40f, 1.0f, 1000.0f);

    CAPTURE (numSamples, amplitude);

    // Sweep the gain across its whole range between blocks, as host automation would
    for (const auto gainDb : { -60.0f, -30.0f, -6.0f, 0.0f, 6.0f })
    {
        CAPTURE (gainDb);
        setParameter (processor, Parameters::masterGainId, gainDb);

        auto buffer = makeSineBuffer (2, numSamples, 440.0f, amplitude);
        process (processor, buffer);

        CHECK (allSamplesFinite (buffer));
    }
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "Only matching mono or stereo layouts are supported", "[buses]")
{
    using Set = juce::AudioChannelSet;

    // Each layout is { { input bus }, { output bus } }
    CHECK (processor.isBusesLayoutSupported ({ { Set::mono() },   { Set::mono() } }));
    CHECK (processor.isBusesLayoutSupported ({ { Set::stereo() }, { Set::stereo() } }));

    CHECK_FALSE (processor.isBusesLayoutSupported ({ { Set::mono() },   { Set::stereo() } }));
    CHECK_FALSE (processor.isBusesLayoutSupported ({ { Set::stereo() }, { Set::mono() } }));
    CHECK_FALSE (processor.isBusesLayoutSupported ({ { Set::create5point1() }, { Set::create5point1() } }));
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "State round trips into a new instance", "[state]")
{
    setParameter (processor, Parameters::masterGainId, -12.0f);

    juce::MemoryBlock state;
    processor.getStateInformation (state);

    AudioPluginAudioProcessor restored;
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));

    CHECK_THAT (getParameter (restored, Parameters::masterGainId), WithinAbs (-12.0, 1.0e-4));
}

TEST_CASE_METHOD (ProcessorFixture, "Invalid state is ignored", "[state]")
{
    setParameter (processor, Parameters::masterGainId, -3.0f);

    SECTION ("Random bytes")
    {
        const char garbage[] = "definitely not plugin state";
        processor.setStateInformation (garbage, static_cast<int> (sizeof (garbage)));
    }

    SECTION ("Empty data")
    {
        processor.setStateInformation (nullptr, 0);
    }

    SECTION ("Valid XML with the wrong root tag")
    {
        juce::MemoryBlock state;
        juce::AudioProcessor::copyXmlToBinary (juce::XmlElement ("SomethingElse"), state);
        processor.setStateInformation (state.getData(), static_cast<int> (state.getSize()));
    }

    CHECK_THAT (getParameter (processor, Parameters::masterGainId), WithinAbs (-3.0, 1.0e-4));
}
