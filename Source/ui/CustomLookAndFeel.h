#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

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
    static constexpr float regularWeight = 400.0f;
    static constexpr float boldWeight = 700.0f;

    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    /** Orbitron at any weight from 400 to 900, from the one variable font in Assets/. */
    [[nodiscard]] static juce::FontOptions orbitron (float weight);

    // Plugins can't set the default LookAndFeel, so getTypefaceForFont never reaches editor text
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getPopupMenuFont() override;

    // Add your custom drawing overrides below, e.g.:
    // void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
    //                        float sliderPosProportional, float rotaryStartAngle,
    //                        float rotaryEndAngle, juce::Slider&) override { ... }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLookAndFeel)
};
