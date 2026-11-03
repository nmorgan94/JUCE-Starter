#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"
#include "PluginEditor.h"
#include "ui/HoverTip.h"

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

TEST_CASE ("A hover tip shows while the mouse is over its target", "[editor]")
{
    juce::Component target;
    HoverTip tip { target, "Tip" };
    CHECK_FALSE (tip.isVisible());

    // Called directly: JUCE only routes mouse events to a window that's on top
    const juce::MouseEvent event (juce::Desktop::getInstance().getMainMouseSource(), {}, {},
                                  juce::MouseInputSource::defaultPressure, 0.0f, 0.0f, 0.0f, 0.0f,
                                  &target, &target, juce::Time::getCurrentTime(), {}, juce::Time::getCurrentTime(),
                                  0, false);

    tip.mouseEnter (event);
    CHECK (tip.isVisible());

    tip.mouseExit (event);
    CHECK_FALSE (tip.isVisible());
}

TEST_CASE_METHOD (ProcessorFixture, "About shows the version and build type, the commit and the UI", "[editor]")
{
    const auto lines = juce::StringArray::fromLines (About::text (processor));

    REQUIRE (lines.size() == 5);
    CHECK (lines[0] == "Version " JucePlugin_VersionString);
   #if JUCE_DEBUG
    CHECK (lines[1] == "Debug build");
   #else
    CHECK (lines[1] == "Release build");
   #endif
    CHECK (lines[2] == juce::String ("Commit ") + About::commit);
    CHECK (lines[3] == "Undefined");
    CHECK (lines[4] == "Native UI");
}

TEST_CASE_METHOD (ProcessorFixture, "The gain knob draws its value in blue with a white dot", "[editor]")
{
    AudioPluginAudioProcessorEditor editor (processor);
    auto* knob = dynamic_cast<juce::Slider*> (editor.findChildWithID (Parameters::masterGainId.getParamID()));
    REQUIRE (knob != nullptr);
    knob->setValue (knob->getMaximum());

    // CustomLookAndFeel::drawRotarySlider's geometry
    const auto bounds = knob->getLookAndFeel().getSliderLayout (*knob).sliderBounds.toFloat().reduced (10.0f);
    const auto centre = bounds.getCentre();
    const auto radius = std::min (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto arcRadius = radius - std::min (8.0f, radius * 0.5f) * 0.5f;
    const auto arc = centre.getPointOnCircumference (arcRadius, juce::MathConstants<float>::halfPi).toInt();
    const auto dot = centre.getPointOnCircumference (arcRadius, knob->getRotaryParameters().endAngleRadians).toInt();

    const auto image = knob->createComponentSnapshot (knob->getLocalBounds());

    using UI = juce::LookAndFeel_V4::ColourScheme::UIColour;
    CHECK (image.getPixelAt (arc.x, arc.y) == juce::LookAndFeel_V4::getDarkColourScheme().getUIColour (UI::defaultFill));
    CHECK (image.getPixelAt (dot.x, dot.y) == knob->findColour (juce::Slider::textBoxTextColourId));
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

    // The title, the GAIN heading and the knob's text box
    REQUIRE (labels.size() == 3);

    for (auto* label : labels)
    {
        CAPTURE (label->getText());
        CHECK (editor.getLookAndFeel().getLabelFont (*label).getTypefacePtr()->getName() == "Orbitron");
    }

    std::vector<juce::TextButton*> buttons;
    for (auto* child : editor.getChildren())
        if (dynamic_cast<PresetBar*> (child) != nullptr || dynamic_cast<ModalOverlay*> (child) != nullptr)
            for (auto* grandchild : child->getChildren())
                if (auto* button = dynamic_cast<juce::TextButton*> (grandchild))
                    buttons.push_back (button);

    // The preset bar's five, the prompt's two and the manual's one
    REQUIRE (buttons.size() == 8);

    for (auto* button : buttons)
    {
        CAPTURE (button->getButtonText());
        CHECK (editor.getLookAndFeel().getTextButtonFont (*button, button->getHeight()).getTypefacePtr()->getName() == "Orbitron");
    }

    CHECK (editor.getLookAndFeel().getPopupMenuFont().getTypefacePtr()->getName() == "Orbitron");
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
