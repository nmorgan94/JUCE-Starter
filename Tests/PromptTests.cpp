#include <catch2/catch_test_macros.hpp>

#include "TestHelpers.h"

using TestHelpers::OverlayFixture;

//==============================================================================
TEST_CASE_METHOD (OverlayFixture, "A prompt covers its parent and runs its callback on Return, not on Escape", "[prompt][ui]")
{
    PromptOverlay prompt;
    parent.addChildComponent (prompt);
    CHECK_FALSE (prompt.isVisible());

    int confirmed = 0;
    const auto confirm = [&] { prompt.askToConfirm ("Delete", "Delete it?", "Delete", [&] { ++confirmed; }); };

    confirm();
    CHECK (prompt.isVisible());
    CHECK (prompt.getBounds() == parent.getLocalBounds());

    CHECK (prompt.keyPressed (juce::KeyPress ('a')));   // swallowed
    CHECK (prompt.isVisible());

    CHECK (prompt.keyPressed (juce::KeyPress (juce::KeyPress::escapeKey)));
    CHECK_FALSE (prompt.isVisible());
    CHECK (confirmed == 0);

    confirm();
    CHECK (prompt.keyPressed (juce::KeyPress (juce::KeyPress::returnKey)));
    CHECK_FALSE (prompt.isVisible());
    CHECK (confirmed == 1);
}

TEST_CASE_METHOD (OverlayFixture, "A text prompt passes its text to the callback", "[prompt][ui]")
{
    PromptOverlay prompt;
    parent.addChildComponent (prompt);

    juce::String received;
    prompt.askForText ("Rename", "Old name", "Rename", [&] (const juce::String& text) { received = text; });
    prompt.keyPressed (juce::KeyPress (juce::KeyPress::returnKey));

    CHECK (received == "Old name");
}
