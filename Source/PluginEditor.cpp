#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&customLookAndFeel);

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

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
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
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();

    outputMeter.setBounds (bounds.removeFromRight (50)
                               .withSizeKeepingCentre (LevelMeter::preferredWidth, 160));

    auto knobArea = bounds.withSizeKeepingCentre (140, 190);
    gainLabel.setBounds (knobArea.removeFromTop (24));
    gainKnob.setBounds (knobArea);
}
