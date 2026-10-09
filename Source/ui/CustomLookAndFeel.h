#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "LevelMeter.h"

//==============================================================================
/**
 * CustomLookAndFeel
 *
 * Override LookAndFeel_V4 methods here to customise the appearance of your plugin.
 * See: https://docs.juce.com/master/classLookAndFeel__V4.html
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Custom components have no colours in LookAndFeel_V4, so give them some here
        const auto& scheme = getCurrentColourScheme();
        setColour (LevelMeter::backgroundColourId, scheme.getUIColour (ColourScheme::UIColour::widgetBackground));
        setColour (LevelMeter::outlineColourId,    scheme.getUIColour (ColourScheme::UIColour::outline));
        setColour (LevelMeter::barColourId,        scheme.getUIColour (ColourScheme::UIColour::defaultFill));
        setColour (LevelMeter::clipColourId,       juce::Colour (0xffff3d4a));
    }

    ~CustomLookAndFeel() override = default;

    // Add your custom drawing overrides below, e.g.:
    // void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
    //                        float sliderPosProportional, float rotaryStartAngle,
    //                        float rotaryEndAngle, juce::Slider&) override { ... }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLookAndFeel)
};
