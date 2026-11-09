#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"
#include "PluginEditor.h"
#include "ui/LabeledSlider.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

template <typename T>
static std::vector<T*> descendantsOf (juce::Component& parent)
{
    std::vector<T*> found;

    for (auto* child : parent.getChildren())
    {
        if (auto* match = dynamic_cast<T*> (child))
            found.push_back (match);

        const auto below = descendantsOf<T> (*child);
        found.insert (found.end(), below.begin(), below.end());
    }

    return found;
}

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

TEST_CASE ("A labelled slider shows its value while it's moved", "[editor]")
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    LabeledSlider control;
    auto& slider = control.getSlider();
    slider.setRange (-60.0, 6.0, 0.1);
    slider.setTextValueSuffix (" dB");
    control.setLabelText ("GAIN");

    const auto labels = descendantsOf<juce::Label> (control);
    REQUIRE (labels.size() == 1);
    auto& label = *labels.front();
    CHECK (label.getText() == "GAIN");
    CHECK (slider.getTitle() == "GAIN");

    slider.setValue (-3.0, juce::sendNotificationSync);
    CHECK (label.getText() == "GAIN");

    {
        juce::Slider::ScopedDragNotification drag (slider);
        slider.setValue (-12.0, juce::sendNotificationSync);
        CHECK (label.getText() == "-12.0 dB");
    }

    // Until the timer brings the name back
    CHECK (label.getText() == "-12.0 dB");

    slider.setValue (-9.0, juce::sendNotificationSync);
    CHECK (label.getText() == "-9.0 dB");

    // As if typed: the label's editor needs it on screen
    label.setText ("-6", juce::sendNotificationSync);
    CHECK_THAT (slider.getValue(), WithinAbs (-6.0, 1.0e-9));
    CHECK (label.getText() == "-6.0 dB");

    struct GestureCounter : juce::Slider::Listener
    {
        int gestures = 0;
        void sliderValueChanged (juce::Slider*) override {}
        void sliderDragStarted (juce::Slider*) override   { ++gestures; }
    } counter;
    slider.addListener (&counter);

    // The same value, then text that doesn't start with a number
    label.setText ("GAIN", juce::dontSendNotification);
    label.setText ("-6.0 dB", juce::sendNotificationSync);
    label.setText ("abc", juce::sendNotificationSync);
    label.setText ("dB 5", juce::sendNotificationSync);
    CHECK (counter.gestures == 0);
    CHECK_THAT (slider.getValue(), WithinAbs (-6.0, 1.0e-9));
    CHECK (label.getText() == "-6.0 dB");

    slider.removeListener (&counter);
}

TEST_CASE ("A labelled control's label goes right under a square control, the pair centred", "[editor]")
{
    // The same cases as webui/test/layout.test.ts
    juce::ScopedJuceInitialiser_GUI juceInitialiser;
    juce::Label label;

    const auto check = [&label] (int width, int height, juce::Rectangle<int> control, juce::Rectangle<int> labelBounds)
    {
        CHECK (LabeledSlider::layOutAboveLabel (label, { width, height }) == control);
        CHECK (label.getBounds() == labelBounds);
    };

    check (140, 190, { 0, 13, 140, 140 }, { 0, 153, 140, 24 });
    check (100, 101, { 11, 0, 77, 77 }, { 0, 77, 100, 24 });
    check (50, 21, { 25, -1, 0, 0 }, { 0, -1, 50, 24 });
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
    auto* knob = findGainKnob (editor);
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

    const auto labels = descendantsOf<juce::Label> (editor);

    // The knob's label, the overlays' titles and the prompt's message
    REQUIRE (labels.size() == 4);

    for (auto* label : labels)
    {
        CAPTURE (label->getText());
        CHECK (editor.getLookAndFeel().getLabelFont (*label).getTypefacePtr()->getName() == "Orbitron");
    }

    const auto buttons = descendantsOf<juce::TextButton> (editor);

    // The preset bar's four, the header's menu button, the prompt's two and the manual's one
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
