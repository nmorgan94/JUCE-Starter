// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo } from 'react';
import type { PluginBackend } from './backend.ts';
import { LabeledSlider } from './controls.tsx';
import { HEADER_HEIGHT, HeaderBar } from './HeaderBar.tsx';
import { centred } from './layout.ts';
import { useManual } from './ManualOverlay.tsx';
import { usePrompt } from './PromptOverlay.tsx';

export const EDITOR_WIDTH = 440;
export const EDITOR_HEIGHT = 312;

const CONTENT_HEIGHT = EDITOR_HEIGHT - HEADER_HEIGHT;

const KNOB_AREA = { width: 140, height: 190 };

const knobLeft = centred(EDITOR_WIDTH, KNOB_AREA.width);
const knobTop = centred(CONTENT_HEIGHT, KNOB_AREA.height);

export const Editor = memo(function Editor({ backend }: { backend: PluginBackend }) {
  const { prompt, overlay } = usePrompt();
  const manual = useManual();

  return (
    <div className="relative bg-background" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT }}>
      <div className="absolute inset-x-0" style={{ top: HEADER_HEIGHT, height: CONTENT_HEIGHT }}>
        <div className="absolute" style={{ left: knobLeft, top: knobTop }}>
          <LabeledSlider backend={backend} id="masterGain" label="GAIN" width={KNOB_AREA.width} height={KNOB_AREA.height} />
        </div>
      </div>
      <HeaderBar title="JUCE STARTER" width={EDITOR_WIDTH} backend={backend} prompt={prompt} showManual={manual.showManual} />
      {overlay}
      {manual.overlay}
    </div>
  );
});
