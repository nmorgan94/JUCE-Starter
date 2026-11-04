# JUCE Starter

A gain effect with a level meter. It works in mono or stereo.

## Master Gain

The GAIN knob sets the output level, from -60 dB to +6 dB. At 0 dB the sound passes through unchanged.
Drag up or down to turn it. While you do, its name shows the value. Click the name to type a value.

## Meter

The meter at the top right shows the output peak, one bar per channel. The light above a bar turns on
when that channel goes over 0 dB and stays on for a moment.

## Presets

- The arrows step through the presets, and the name opens the full list.
- A * after the name means the sound has changed since the preset was loaded.
- Save overwrites the current user preset. For a factory preset it asks for a name.
- The ... menu has Init Preset, Save As, Rename and Delete. In the plugin it also has Show Preset Folder.
- Factory presets can't be renamed, deleted or overwritten.
