#pragma once

#include "LevelMeter.h"
#include "PresetActions.h"
#include "PresetBar.h"

//==============================================================================
/**
 * The strip along the top of the editor: the title, the preset bar, the menu and the output meter.
 */
class HeaderBar : public juce::Component
{
public:
    static constexpr int height = 44;

    HeaderBar (const juce::String& title, PresetManager&, PromptOverlay&,
               std::function<void()> showManual, const juce::String& aboutText, PeakMeter& meter);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PromptOverlay& prompt;
    const std::function<void()> showManual;
    const juce::String aboutText;

    juce::Label title;
    PresetActions presetActions;
    PresetBar presetBar;
    juce::TextButton menuButton { "..." };
    LevelMeter meter;

    void showMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderBar)
};
