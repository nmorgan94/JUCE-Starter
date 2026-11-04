#pragma once

#include "PluginProcessor.h"
#include "ui/About.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/HeaderBar.h"
#include "ui/LabeledSlider.h"
#include "ui/Manual.h"
#include "ui/PromptOverlay.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    static constexpr int width = 440, height = 312;

    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    CustomLookAndFeel customLookAndFeel;

    PromptOverlay prompt;
    ManualOverlay manual;
    HeaderBar header { "JUCE STARTER", processorRef.getPresetManager(), prompt, [this] { manual.show(); },
                       About::text (processorRef), processorRef.getOutputMeter() };

    LabeledSlider gainKnob;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment {
        processorRef.getAPVTS(), Parameters::masterGainId.getParamID(), gainKnob.getSlider() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
