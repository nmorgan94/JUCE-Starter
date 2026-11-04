#include "HeaderBar.h"
#include "CustomLookAndFeel.h"

HeaderBar::HeaderBar (const juce::String& titleText, PresetManager& presets, PromptOverlay& o,
                      std::function<void()> m, const juce::String& about, PeakMeter& source)
    : prompt (o), showManual (std::move (m)), aboutText (about),
      presetActions (presets, prompt), presetBar (presets, presetActions), meter (source)
{
    title.setText (titleText, juce::dontSendNotification);
    title.setFont (CustomLookAndFeel::orbitron (CustomLookAndFeel::boldWeight).withHeight (18.0f));
    title.setJustificationType (juce::Justification::centredLeft);
    title.setBorderSize ({});

    menuButton.onClick = [this] { showMenu(); };

    addAndMakeVisible (title);
    addAndMakeVisible (presetBar);
    addAndMakeVisible (menuButton);
    addAndMakeVisible (meter);
}

void HeaderBar::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::Slider::backgroundColourId));
}

void HeaderBar::resized()
{
    const auto bounds = getLocalBounds();

    const auto titleWidth = juce::GlyphArrangement::getStringWidth (title.getFont(), title.getText());
    title.setBounds (bounds.withTrimmedLeft (12).withWidth (static_cast<int> (std::ceil (titleWidth))));
    auto bar = bounds.withTrimmedRight (12);
    meter.setBounds (bar.removeFromRight (LevelMeter::preferredWidth).reduced (0, 4));

    auto controls = bar.withLeft (152).withTrimmedRight (12).reduced (0, 10);
    menuButton.setBounds (controls.removeFromRight (28));
    controls.removeFromRight (4);
    presetBar.setBounds (controls);
}

void HeaderBar::showMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel (&getLookAndFeel());
    presetActions.addMenuItems (menu);
    menu.addSeparator();
    menu.addItem ("Manual", showManual);
    menu.addItem ("About", [this] { prompt.showMessage ("About", aboutText); });

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (menuButton));
}
