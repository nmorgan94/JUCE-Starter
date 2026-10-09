#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
namespace PluginConfig
{
    enum class Channels { mono, stereo };

    // Set once per plugin. Decides the bus layout hosts can load the plugin with, and how
    // many bars the level meter shows.
    inline constexpr auto channels = Channels::stereo;

    inline constexpr int numChannels = channels == Channels::mono ? 1 : 2;

    inline juce::AudioChannelSet channelSet()
    {
        return numChannels == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo();
    }
}
