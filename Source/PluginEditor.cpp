#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&customLookAndFeel);

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

    g.setColour (juce::Colours::white);
    
    // Create a centered text block
    auto bounds = getLocalBounds();
    int blockWidth = bounds.getWidth() - 100;
    int blockHeight = bounds.getHeight() - 60;
    auto textBlock = bounds.withSizeKeepingCentre(blockWidth, blockHeight);
    
    // Draw title
    g.setFont (juce::FontOptions(16.0f).withStyle("Bold"));
    auto titleArea = textBlock.removeFromTop(30);
    g.drawText ("Welcome to the JUCE Starter!", titleArea, juce::Justification::centredTop);
    
    textBlock.removeFromTop(10); // Add spacing
    
    // Draw instructions with left-aligned bullets
    g.setFont (juce::FontOptions(15.0f));
    juce::String instructions = "If you are using binary assets:\n\n"
                                "- Add them to the Assets directory\n"
                                "- Add BinaryData header to the Plugin Editor\n"
                                "- Reconfigure and build the Plugin with CMAKE";
    
    g.drawFittedText (instructions, textBlock, juce::Justification::topLeft, 10);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

}
