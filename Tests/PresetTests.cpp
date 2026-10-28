#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

namespace
{
    // The plugin's factory presets, with user presets in a folder of their own
    struct PresetFixture : ProcessorFixture
    {
        ~PresetFixture()   { userDirectory.deleteRecursively(); }

        juce::File userDirectory = juce::File::createTempFile ("presets");

        PresetManager presets { processor.getAPVTS(), [this]
        {
            auto config = AudioPluginAudioProcessor::createPresetConfig();
            config.userDirectory = userDirectory;
            return config;
        }() };

        float gain()   { return getParameter (processor, Parameters::masterGainId); }
    };

    juce::StringArray namesOf (const juce::Array<PresetManager::Preset>& list)
    {
        juce::StringArray names;

        for (const auto& preset : list)
            names.add (preset.name);

        return names;
    }

    // The same order as webui/test/demo/localPresets.test.ts
    const juce::StringArray factoryNames { "Boost +3", "Boost +6", "Cut -6", "Cut -12", "Cut -24" };
}

//==============================================================================
TEST_CASE_METHOD (PresetFixture, "Factory presets are listed by category, then name", "[presets]")
{
    CHECK (namesOf (presets.getFactoryPresets()) == factoryNames);
    CHECK (namesOf (presets.getPresets()) == factoryNames);

    for (const auto& preset : presets.getFactoryPresets())
    {
        CAPTURE (preset.name);
        CHECK (preset.isFactory);
        CHECK (preset.name.startsWith (preset.category));
    }
}

TEST_CASE_METHOD (PresetFixture, "A loaded preset sets its values and shows its name", "[presets]")
{
    CHECK (presets.getCurrent().name == "Init");

    REQUIRE (presets.loadPreset (presets.find ("Cut -12", true).value()));

    CHECK_THAT (gain(), WithinAbs (-12.0, 1.0e-4));
    CHECK (presets.getCurrent().name == "Cut -12");
    CHECK (presets.getCurrent().category == "Cut");
    CHECK (presets.getCurrent().isFactory);
    CHECK_FALSE (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "Moving a parameter marks the preset modified until the next load", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));

    setParameter (processor, Parameters::masterGainId, -1.0f);
    CHECK (presets.isModified());

    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));
    CHECK_FALSE (presets.isModified());
    CHECK_THAT (gain(), WithinAbs (3.0, 1.0e-4));
}

TEST_CASE_METHOD (PresetFixture, "Init restores the defaults", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +6", true).value()));
    setParameter (processor, Parameters::masterGainId, 4.0f);

    presets.loadInit();

    CHECK_THAT (gain(), WithinAbs (ParameterSpecs::masterGain.defaultValue, 1.0e-4));
    CHECK (presets.getCurrent().name == "Init");
    CHECK_FALSE (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "Next and previous wrap, and start from either end after Init", "[presets]")
{
    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);

    REQUIRE (presets.loadPrevious());
    CHECK (presets.getCurrent().name == factoryNames[factoryNames.size() - 1]);

    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);

    presets.loadInit();
    REQUIRE (presets.loadPrevious());
    CHECK (presets.getCurrent().name == factoryNames[factoryNames.size() - 1]);
    CHECK_THAT (gain(), WithinAbs (-24.0, 1.0e-4));
}

TEST_CASE_METHOD (PresetFixture, "User presets save, rename and delete in their folder", "[presets]")
{
    setParameter (processor, Parameters::masterGainId, -3.0f);

    REQUIRE (presets.saveUserPreset ("Mine", "Cut").wasOk());
    const auto mine = userDirectory.getChildFile ("Mine.srstarter");
    CHECK (mine.existsAsFile());
    CHECK (presets.getCurrent().name == "Mine");
    CHECK_FALSE (presets.getCurrent().isFactory);
    CHECK_FALSE (presets.isModified());

    // Listed after the factory presets
    REQUIRE (presets.getPresets().getLast().file == mine);

    presets.loadInit();
    REQUIRE (presets.loadPreset (presets.getPresets().getLast()));
    CHECK_THAT (gain(), WithinAbs (-3.0, 1.0e-4));
    CHECK (presets.getCurrent().name == "Mine");

    REQUIRE (presets.renameUserPreset (presets.getPresets().getLast(), "Yours").wasOk());
    const auto yours = userDirectory.getChildFile ("Yours.srstarter");
    CHECK_FALSE (mine.exists());
    CHECK (yours.existsAsFile());
    CHECK (presets.getCurrent().name == "Yours");
    CHECK (namesOf (presets.getUserPresets()) == juce::StringArray { "Yours" });

    REQUIRE (presets.deleteUserPreset (presets.getPresets().getLast()).wasOk());
    CHECK_FALSE (yours.exists());
    CHECK (presets.getUserPresets().isEmpty());
    CHECK (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "User presets can't take a factory name", "[presets]")
{
    CHECK (presets.saveUserPreset ("boost +3", {}).failed());
    CHECK (presets.saveUserPreset ("   ", {}).failed());
    CHECK_FALSE (userDirectory.getChildFile ("boost +3.srstarter").exists());

    REQUIRE (presets.saveUserPreset ("Mine", {}).wasOk());
    CHECK (presets.renameUserPreset (presets.getUserPresets().getFirst(), "Cut -6").failed());
    CHECK (userDirectory.getChildFile ("Mine.srstarter").existsAsFile());
}

TEST_CASE_METHOD (PresetFixture, "Next and previous skip a user preset deleted outside the plugin", "[presets]")
{
    REQUIRE (presets.saveUserPreset ("A", {}).wasOk());
    REQUIRE (presets.saveUserPreset ("B", {}).wasOk());
    REQUIRE (presets.loadPreset (presets.find ("A", false).value()));

    REQUIRE (userDirectory.getChildFile ("B.srstarter").deleteFile());

    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);
}

TEST_CASE_METHOD (PresetFixture, "Presets are found by name and kind", "[presets]")
{
    REQUIRE (presets.saveUserPreset ("Mine", {}).wasOk());

    CHECK (presets.find ("Cut -6", true).has_value());
    CHECK (presets.find ("Mine", false)->file == userDirectory.getChildFile ("Mine.srstarter"));
    CHECK_FALSE (presets.find ("Cut -6", false).has_value());
    CHECK_FALSE (presets.find ("Mine", true).has_value());
}

TEST_CASE_METHOD (PresetFixture, "State from before presets existed shows Init, modified", "[presets][state]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +6", true).value()));

    juce::ValueTree state (processor.getAPVTS().state.getType());
    state.appendChild ({ "PARAM", { { "id", Parameters::masterGainId.getParamID() }, { "value", -20.0f } } }, nullptr);
    presets.restoreState (state);

    CHECK_THAT (gain(), WithinAbs (-20.0, 1.0e-4));
    CHECK (presets.getCurrent().name == "Init");
    CHECK (presets.isModified());
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
