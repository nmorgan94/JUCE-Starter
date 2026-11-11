// PresetManager's own tests are in SampleRealm-Modules. These are this plugin's presets and state
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

namespace
{
    // The same order as webui/test/demo/factoryPresets.test.ts
    const juce::StringArray factoryNames { "Boost +3", "Boost +6", "Cut -6", "Cut -12", "Cut -24" };
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "The factory presets are embedded and set this plugin's parameters", "[presets]")
{
    auto& presets = processor.getPresetManager();
    juce::StringArray names;

    for (const auto& preset : presets.getFactoryPresets())
        names.add (preset.name);

    CHECK (names == factoryNames);

    REQUIRE (presets.loadPreset (presets.find ("Cut -12", true).value()));
    CHECK_THAT (getParameter (processor, Parameters::masterGainId), WithinAbs (-12.0, 1.0e-4));
}

TEST_CASE_METHOD (ProcessorFixture, "A reopened session shows the same preset", "[presets][state]")
{
    auto& presets = processor.getPresetManager();
    REQUIRE (presets.loadPreset (presets.find ("Boost +6", true).value()));
    setParameter (processor, Parameters::masterGainId, 2.0f);

    juce::MemoryBlock state;
    processor.getStateInformation (state);

    AudioPluginAudioProcessor restored;
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));

    const auto current = restored.getPresetManager().getCurrent();
    CHECK (current.name == "Boost +6");
    CHECK (current.category == "Boost");
    CHECK (current.isFactory);
    CHECK (restored.getPresetManager().isModified());
    CHECK_THAT (getParameter (restored, Parameters::masterGainId), WithinAbs (2.0, 1.0e-4));

    // Kept by the preset manager, not left on the parameters
    CHECK_FALSE (restored.getAPVTS().state.hasProperty ("presetName"));
}
