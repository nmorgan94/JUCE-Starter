#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&customLookAndFeel);

    title.setComponentID ("title");
    title.setText ("JUCE STARTER", juce::dontSendNotification);
    title.setFont (CustomLookAndFeel::orbitron (CustomLookAndFeel::boldWeight).withHeight (18.0f));
    title.setJustificationType (juce::Justification::centredLeft);
    title.setBorderSize ({});
    addAndMakeVisible (title);
    addAndMakeVisible (presetBar);

    gainLabel.setText ("GAIN", juce::dontSendNotification);
    gainLabel.setFont (CustomLookAndFeel::orbitron (CustomLookAndFeel::boldWeight).withHeight (16.0f));
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    // The id lets tests find the knob
    gainKnob.setComponentID (Parameters::masterGainId.getParamID());
    gainKnob.setTextValueSuffix (juce::String (" ") + ParameterSpecs::masterGain.unit);
    gainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
    addAndMakeVisible (gainKnob);

    addAndMakeVisible (outputMeter);

    versionTip.setComponentID ("versionTip");
    addChildComponent (versionTip);

    prompt.setFont (CustomLookAndFeel::orbitron (CustomLookAndFeel::regularWeight).withHeight (14.0f));
    addChildComponent (prompt);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (width, height);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (findColour (juce::Slider::backgroundColourId));
    g.fillRect (getLocalBounds().removeFromTop (titleHeight));
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();

    const auto titleBar = bounds.removeFromTop (titleHeight);
    const auto titleWidth = juce::GlyphArrangement::getStringWidth (title.getFont(), title.getText());
    title.setBounds (titleBar.withTrimmedLeft (12).withWidth (static_cast<int> (std::ceil (titleWidth))));
    versionTip.setBounds (title.getX(), titleBar.getBottom() + 4, versionTip.getIdealWidth(), 22);
    auto bar = titleBar.withTrimmedRight (12);
    outputMeter.setBounds (bar.removeFromRight (LevelMeter::preferredWidth).reduced (0, 4));
    presetBar.setBounds (bar.withLeft (152).withTrimmedRight (12).reduced (0, 10));

    auto knobArea = bounds.withSizeKeepingCentre (140, 190);
    gainLabel.setBounds (knobArea.removeFromTop (24));
    gainKnob.setBounds (knobArea);
}
