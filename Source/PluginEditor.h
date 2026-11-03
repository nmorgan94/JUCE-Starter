#pragma once

#include "PluginProcessor.h"
#include "ui/About.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/LevelMeter.h"
#include "ui/Manual.h"
#include "ui/PresetBar.h"
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
    static constexpr int titleHeight = 44;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    CustomLookAndFeel customLookAndFeel;

    juce::Label title;

    PromptOverlay prompt;
    ManualOverlay manual;
    PresetBar presetBar { processorRef.getPresetManager(), prompt, [this] { manual.show(); }, About::text (processorRef) };

    juce::Label gainLabel;
    juce::Slider gainKnob { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment {
        processorRef.getAPVTS(), Parameters::masterGainId.getParamID(), gainKnob };

    LevelMeter outputMeter { processorRef.getOutputMeter() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
