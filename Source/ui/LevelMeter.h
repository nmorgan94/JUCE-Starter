#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/PeakMeter.h"

//==============================================================================
/** A narrow peak meter with one bar per channel, each with a clip light on top. Bars rise at
    once and fall smoothly, and clip lights hold for a moment. */
class LevelMeter : public juce::Component,
                   private juce::Timer
{
public:
    enum ColourIds
    {
        backgroundColourId = 0x5d10300,
        outlineColourId,
        barColourId,
        clipColourId
    };

    //==============================================================================
    /** What one bar shows. update() is called at updateHz with the peak since the last call. */
    struct Ballistics
    {
        static constexpr int updateHz = 30;
        static constexpr int clipHoldTicks = 2 * updateHz;
        static constexpr float floorDb = -60.0f;
        static constexpr float fallDbPerTick = 20.0f / updateHz;

        float levelDb = floorDb;
        int clipTicks = 0;

        bool isClipping() const noexcept   { return clipTicks > 0; }

        /** Takes a peak as linear gain. Returns true if what the bar shows has changed. */
        bool update (float peak) noexcept
        {
            const auto wasClipping = isClipping();

            if (peak > 1.0f)
                clipTicks = clipHoldTicks;
            else if (clipTicks > 0)
                --clipTicks;

            // Capped at the top of the scale, so even an infinite peak can still fall away
            const auto newLevel = juce::jmin (0.0f, std::max (juce::Decibels::gainToDecibels (peak, floorDb),
                                                              levelDb - fallDbPerTick));

            const auto changed = ! juce::approximatelyEqual (newLevel, levelDb) || wasClipping != isClipping();
            levelDb = newLevel;
            return changed;
        }
    };

    //==============================================================================
    explicit LevelMeter (PeakMeter& source) : meter (source)
    {
        // Drop anything played while no meter was showing
        for (int channel = 0; channel < meter.getNumChannels(); ++channel)
            meter.readAndReset (channel);

        startTimerHz (Ballistics::updateHz);
    }

    void paint (juce::Graphics& g) override
    {
        const auto numChannels = meter.getNumChannels();
        auto area = getLocalBounds().toFloat();
        const auto barWidth = (area.getWidth() - gap * static_cast<float> (numChannels - 1))
                              / static_cast<float> (numChannels);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            paintBar (g, area.removeFromLeft (barWidth), bars[static_cast<size_t> (channel)]);
            area.removeFromLeft (gap);
        }
    }

private:
    static constexpr float gap = 3.0f;

    PeakMeter& meter;
    std::array<Ballistics, PeakMeter::maxChannels> bars;

    void paintBar (juce::Graphics& g, juce::Rectangle<float> bounds, const Ballistics& bar) const
    {
        bounds = bounds.reduced (0.5f);
        const auto light = bounds.removeFromTop (6.0f);
        bounds.removeFromTop (3.0f);

        g.setColour (findColour (bar.isClipping() ? clipColourId : outlineColourId));
        g.fillRoundedRectangle (light, 2.0f);

        g.setColour (findColour (backgroundColourId));
        g.fillRoundedRectangle (bounds, 2.0f);

        const auto fraction = juce::jmap (bar.levelDb, Ballistics::floorDb, 0.0f, 0.0f, 1.0f);
        auto fill = bounds.reduced (1.0f);
        g.setColour (findColour (barColourId));
        g.fillRoundedRectangle (fill.removeFromBottom (fill.getHeight() * fraction), 1.5f);

        g.setColour (findColour (outlineColourId));
        g.drawRoundedRectangle (bounds, 2.0f, 1.0f);
    }

    void timerCallback() override
    {
        auto changed = false;

        for (int channel = 0; channel < meter.getNumChannels(); ++channel)
            changed |= bars[static_cast<size_t> (channel)].update (meter.readAndReset (channel));

        if (changed)
            repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
