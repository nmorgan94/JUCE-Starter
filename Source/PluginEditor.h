#pragma once

#include "PluginProcessor.h"
#include "ui/CustomLookAndFeel.h"
#include <BinaryData.h>

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    static constexpr int width = 440, height = 312;

    // As HEADER_LAYOUT in webui/src/Editor.tsx
    static constexpr HeaderBar::Layout headerLayout { .height = 44, .edge = 12, .titleWidth = 128, .titleHeight = 18.0f,
                                                      .gap = 12, .controlsHeight = 24, .meterWidth = LevelMeter::preferredWidth, .meterInset = 4 };

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
    ManualOverlay manual { juce::String::createStringFromData (BinaryData::Manual_md, BinaryData::Manual_mdSize) };
    LevelMeter outputMeter { processorRef.getOutputMeter() };
    HeaderBar header { "JUCE STARTER", headerLayout, processorRef.getPresetManager(), prompt, [this] { manual.show(); },
                       About::text (processorRef), outputMeter };

    LabeledSlider gainKnob;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment {
        processorRef.getAPVTS(), Parameters::masterGainId.getParamID(), gainKnob.getSlider() };

    // Only in a build that requires activation
    std::unique_ptr<ActivationBar> activationBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
