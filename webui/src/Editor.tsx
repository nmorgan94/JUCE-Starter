// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo } from 'react';
import type { PluginBackend } from './backend.ts';
import { Rotary } from './controls.tsx';
import { juceHeight } from './fonts.ts';
import { HEADER_HEIGHT, HeaderBar } from './HeaderBar.tsx';
import { useManual } from './ManualOverlay.tsx';
import { usePrompt } from './PromptOverlay.tsx';

export const EDITOR_WIDTH = 440;
export const EDITOR_HEIGHT = 312;

const CONTENT_HEIGHT = EDITOR_HEIGHT - HEADER_HEIGHT;

const KNOB_AREA = { width: 140, height: 190 };
const LABEL_HEIGHT = 24;

// juce::Rectangle::withSizeKeepingCentre
const centred = (outer: number, inner: number) => Math.trunc((outer - inner) / 2);

const knobLeft = centred(EDITOR_WIDTH, KNOB_AREA.width);
const knobTop = centred(CONTENT_HEIGHT, KNOB_AREA.height);

export const Editor = memo(function Editor({ backend }: { backend: PluginBackend }) {
  const { prompt, overlay } = usePrompt();
  const manual = useManual();

  return (
    <div className="relative bg-background" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT }}>
      <div className="absolute inset-x-0" style={{ top: HEADER_HEIGHT, height: CONTENT_HEIGHT }}>
        <div className="absolute" style={{ left: knobLeft, top: knobTop, width: KNOB_AREA.width }}>
          <div className="flex items-center justify-center font-bold" style={{ height: LABEL_HEIGHT, fontSize: juceHeight(16) }}>
            GAIN
          </div>
          <Rotary backend={backend} id="masterGain" width={KNOB_AREA.width} height={KNOB_AREA.height - LABEL_HEIGHT} textBox={{ width: 80, height: 22 }} />
        </div>
      </div>
      <HeaderBar title="JUCE STARTER" width={EDITOR_WIDTH} backend={backend} prompt={prompt} showManual={manual.showManual} />
      {overlay}
      {manual.overlay}
    </div>
  );
});
