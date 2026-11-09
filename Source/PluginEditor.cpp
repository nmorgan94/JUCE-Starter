#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&customLookAndFeel);

    addAndMakeVisible (header);

    // The id lets tests find the knob
    gainKnob.setComponentID (Parameters::masterGainId.getParamID());
    gainKnob.setLabelText ("GAIN");
    gainKnob.getSlider().setTextValueSuffix (juce::String (" ") + ParameterSpecs::masterGain.unit);
    addAndMakeVisible (gainKnob);

    prompt.setFont (CustomLookAndFeel::orbitron (CustomLookAndFeel::regularWeight).withHeight (14.0f));
    addChildComponent (prompt);
    addChildComponent (manual);

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
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();

    header.setBounds (bounds.removeFromTop (headerLayout.height));

    gainKnob.setBounds (bounds.withSizeKeepingCentre (140, 190));
}
