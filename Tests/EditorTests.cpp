#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"
#include "PluginEditor.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "The gain knob and the parameter follow each other", "[editor]")
{
    AudioPluginAudioProcessorEditor editor (processor);
    auto* knob = dynamic_cast<juce::Slider*> (editor.findChildWithID (Parameters::masterGainId.getParamID()));
    REQUIRE (knob != nullptr);

    knob->setValue (-12.0, juce::sendNotificationSync);
    CHECK_THAT (getParameter (processor, Parameters::masterGainId), WithinAbs (-12.0, 1.0e-4));

    setParameter (processor, Parameters::masterGainId, 3.0f);
    CHECK_THAT (knob->getValue(), WithinAbs (3.0, 1.0e-4));
    CHECK (knob->getTextFromValue (knob->getValue()) == "3.0 dB");
}

TEST_CASE_METHOD (ProcessorFixture, "The editor's text is Orbitron", "[editor][font]")
{
    AudioPluginAudioProcessorEditor editor (processor);
    auto* knob = editor.findChildWithID (Parameters::masterGainId.getParamID());
    REQUIRE (knob != nullptr);

    std::vector<juce::Label*> labels;
    for (auto* child : editor.getChildren())
        if (auto* label = dynamic_cast<juce::Label*> (child))
            labels.push_back (label);
    for (auto* child : knob->getChildren())
        if (auto* label = dynamic_cast<juce::Label*> (child))
            labels.push_back (label);

    // The GAIN heading and the knob's text box
    REQUIRE (labels.size() == 2);

    for (auto* label : labels)
    {
        CAPTURE (label->getText());
        CHECK (editor.getLookAndFeel().getLabelFont (*label).getTypefacePtr()->getName() == "Orbitron");
    }
}

TEST_CASE ("Orbitron's weights come from the one variable font", "[editor][font]")
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    const auto weightOf = [] (const juce::FontOptions& options)
    {
        const auto variables = options.getTypeface()->getConfiguredVariables();
        REQUIRE (variables.size() == 1);
        CHECK (variables.front().tag == juce::FontFeatureTag ("wght"));
        return variables.front().value;
    };

    const auto regular = CustomLookAndFeel::orbitron (CustomLookAndFeel::regularWeight);
    const auto bold = CustomLookAndFeel::orbitron (CustomLookAndFeel::boldWeight);

    CHECK (regular.getTypeface()->getName() == "Orbitron");
    CHECK_THAT (weightOf (regular), WithinAbs (400.0, 0.0));
    CHECK_THAT (weightOf (bold), WithinAbs (700.0, 0.0));
}
