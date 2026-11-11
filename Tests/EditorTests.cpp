#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"
#include "PluginEditor.h"
#include <EditorFixtures.h>

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;
using EditorFixtures::descendantsOf;

static juce::Slider* findGainKnob (AudioPluginAudioProcessorEditor& editor)
{
    auto* gain = dynamic_cast<LabeledSlider*> (editor.findChildWithID (Parameters::masterGainId.getParamID()));
    REQUIRE (gain != nullptr);
    return &gain->getSlider();
}

//==============================================================================
TEST_CASE_METHOD (ProcessorFixture, "The gain knob and the parameter follow each other", "[editor]")
{
    AudioPluginAudioProcessorEditor editor (processor);
    auto* knob = findGainKnob (editor);

    knob->setValue (-12.0, juce::sendNotificationSync);
    CHECK_THAT (getParameter (processor, Parameters::masterGainId), WithinAbs (-12.0, 1.0e-4));

    setParameter (processor, Parameters::masterGainId, 3.0f);
    CHECK_THAT (knob->getValue(), WithinAbs (3.0, 1.0e-4));
    CHECK (knob->getTextFromValue (knob->getValue()) == "3.0 dB");
}

TEST_CASE_METHOD (ProcessorFixture, "The title fits the header without being cut short", "[editor]")
{
    AudioPluginAudioProcessorEditor editor (processor);
    const auto headers = descendantsOf<HeaderBar> (editor);
    REQUIRE (headers.size() == 1);

    const auto& layout = AudioPluginAudioProcessorEditor::headerLayout;
    const auto width = juce::GlyphArrangement::getStringWidth (headers[0]->titleFont(), headers[0]->getTitle());

    // drawFittedText only squeezes a title that's narrower than this
    CHECK (width * HeaderBar::minimumTitleScale < layout.titleWidth);
}

TEST_CASE_METHOD (ProcessorFixture, "The editor has an activation bar only in a build that requires activation", "[editor][activation]")
{
    AudioPluginAudioProcessorEditor editor (processor);
    const auto bars = descendantsOf<ActivationBar> (editor);

    REQUIRE (bars.size() == (AudioPluginAudioProcessor::requiresActivation ? 1 : 0));

    if (! bars.empty())
        CHECK (bars[0]->getBounds() == editor.getLocalBounds().removeFromBottom (ActivationBar::height));
}

TEST_CASE_METHOD (ProcessorFixture, "The editor's text is Orbitron", "[editor][font]")
{
    AudioPluginAudioProcessorEditor editor (processor);

    const auto headers = descendantsOf<HeaderBar> (editor);
    REQUIRE (headers.size() == 1);
    CHECK (headers[0]->titleFont().getTypefacePtr()->getName() == "Orbitron");

    const auto labels = descendantsOf<juce::Label> (editor);

    const auto hasActivationBar = AudioPluginAudioProcessor::requiresActivation;

    // The knob's label, the overlays' titles and the prompt's message, and the activation bar's code
    REQUIRE (labels.size() == (hasActivationBar ? 5 : 4));

    for (auto* label : labels)
    {
        CAPTURE (label->getText());
        CHECK (editor.getLookAndFeel().getLabelFont (*label).getTypefacePtr()->getName() == "Orbitron");
    }

    const auto buttons = descendantsOf<juce::TextButton> (editor);

    // The preset bar's four, the header's menu button, the prompt's two and the manual's one,
    // and the activation bar's three
    REQUIRE (buttons.size() == (hasActivationBar ? 11 : 8));

    for (auto* button : buttons)
    {
        CAPTURE (button->getButtonText());
        CHECK (editor.getLookAndFeel().getTextButtonFont (*button, button->getHeight()).getTypefacePtr()->getName() == "Orbitron");
    }

    CHECK (editor.getLookAndFeel().getPopupMenuFont().getTypefacePtr()->getName() == "Orbitron");
}
