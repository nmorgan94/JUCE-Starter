#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <licensing/licensing.h>
#include <samplerealm/PeakMeter.h>
#include "Parameters.h"
#include "dsp/Effect.h"
#include <editor/editor.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void numChannelsChanged() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    PeakMeter& getOutputMeter()                    { return outputMeter; }
    PresetManager& getPresetManager()              { return presetManager; }

    static PresetManager::Config createPresetConfig();

    // Editors show the activation bar only in a build that asks for it
    static constexpr bool requiresActivation = PLUGIN_REQUIRE_ACTIVATION;

    // Made on first use, by an editor: it reads the licence file
    licensing::Activation& getActivation();

private:
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters",
                                               Parameters::createLayout() };
    PresetManager presetManager { apvts, createPresetConfig() };

    Effect effect;

    PeakMeter outputMeter { getTotalNumOutputChannels() };

    std::unique_ptr<licensing::Activation> activation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
