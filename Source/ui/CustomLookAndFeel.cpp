#include "CustomLookAndFeel.h"
#include "LevelMeter.h"
#include "PromptOverlay.h"
#include <BinaryData.h>
#include <map>

namespace
{
    // FontOptions can't take both a typeface and variable settings, so each weight is its own clone
    juce::Typeface::Ptr orbitronTypeface (float weight)
    {
        static const auto variable = juce::Typeface::createSystemTypefaceFor (
            BinaryData::OrbitronVariable_ttf, BinaryData::OrbitronVariable_ttfSize);

        static juce::CriticalSection lock;
        static std::map<float, juce::Typeface::Ptr> byWeight;

        const juce::ScopedLock scope (lock);
        auto& typeface = byWeight[weight];

        if (typeface == nullptr)
        {
            const juce::FontVariableSetting setting { "wght", weight };
            typeface = variable->cloneWithVariableSettings ({ &setting, 1 });
        }

        return typeface;
    }
}

//==============================================================================
CustomLookAndFeel::CustomLookAndFeel()
{
    using UI = ColourScheme::UIColour;
    const auto scheme = getCurrentColourScheme();

    setColour (PromptOverlay::dimColourId, scheme.getUIColour (UI::windowBackground).withAlpha (0.75f));
    setColour (PromptOverlay::panelColourId, scheme.getUIColour (UI::widgetBackground));
    setColour (PromptOverlay::outlineColourId, scheme.getUIColour (UI::outline));
    setColour (LevelMeter::backgroundColourId, scheme.getUIColour (UI::windowBackground));

    setColour (juce::Slider::rotarySliderFillColourId, scheme.getUIColour (UI::defaultFill));
}

juce::FontOptions CustomLookAndFeel::orbitron (float weight)
{
    return juce::FontOptions (orbitronTypeface (weight));
}

juce::Font CustomLookAndFeel::getLabelFont (juce::Label& label)
{
    const auto font = label.getFont();

    // Labels given their own typeface, like the GAIN heading, keep it
    if (font.getTypefaceName() != juce::Font::getDefaultSansSerifFontName())
        return font;

    return juce::Font (orbitron (font.isBold() ? boldWeight : regularWeight).withHeight (font.getHeight()));
}

juce::Font CustomLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (orbitron (regularWeight).withHeight (12.0f));
}

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    return juce::Font (orbitron (regularWeight).withHeight (13.0f));
}

// LookAndFeel_V4's geometry
void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10.0f);
    const auto centre = bounds.getCentre();
    const auto radius = std::min (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto lineWidth = std::min (8.0f, radius * 0.5f);
    const auto arcRadius = radius - lineWidth * 0.5f;
    const auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    const auto strokeArc = [&] (float from, float to)
    {
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, from, to, true);
        g.strokePath (arc, { lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    };

    g.setColour (slider.findColour (juce::Slider::rotarySliderOutlineColourId));
    strokeArc (rotaryStartAngle, rotaryEndAngle);

    if (slider.isEnabled() && sliderPosProportional > 0.0f)
    {
        g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
        strokeArc (rotaryStartAngle, toAngle);
    }

    const auto dotRadius = lineWidth * 0.35f;
    g.setColour (slider.findColour (juce::Slider::textBoxTextColourId));
    g.fillEllipse (juce::Rectangle<float> (dotRadius * 2.0f, dotRadius * 2.0f)
                       .withCentre (centre.getPointOnCircumference (arcRadius, toAngle)));
}
