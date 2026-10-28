#include "CustomLookAndFeel.h"
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
