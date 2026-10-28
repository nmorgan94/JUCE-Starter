// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo } from 'react';
import type { PluginBackend } from './backend.ts';
import { Rotary } from './controls.tsx';
import { juceHeight } from './fonts.ts';
import { LevelMeter, PREFERRED_WIDTH } from './LevelMeter.tsx';
import { HoverTip } from './HoverTip.tsx';
import { PresetBar } from './PresetBar.tsx';
import { usePrompt } from './PromptOverlay.tsx';

export const EDITOR_WIDTH = 440;
export const EDITOR_HEIGHT = 312;

const TITLE_HEIGHT = 44;
const TITLE = { left: 12, top: 0, height: TITLE_HEIGHT };
const TIP = { left: TITLE.left, top: TITLE_HEIGHT + 4, height: 22 };
const METER = { left: EDITOR_WIDTH - 12 - PREFERRED_WIDTH, top: 4, width: PREFERRED_WIDTH, height: TITLE_HEIGHT - 8 };
const PRESET_BAR = { left: 152, top: 10, width: METER.left - 12 - 152, height: TITLE_HEIGHT - 20 };
const CONTENT_HEIGHT = EDITOR_HEIGHT - TITLE_HEIGHT;

const KNOB_AREA = { width: 140, height: 190 };
const LABEL_HEIGHT = 24;

// juce::Rectangle::withSizeKeepingCentre
const centred = (outer: number, inner: number) => Math.trunc((outer - inner) / 2);

const knobLeft = centred(EDITOR_WIDTH, KNOB_AREA.width);
const knobTop = centred(CONTENT_HEIGHT, KNOB_AREA.height);

export const Editor = memo(function Editor({ backend }: { backend: PluginBackend }) {
  const { prompt, overlay } = usePrompt();

  return (
    <div className="relative bg-background" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT }}>
      <div className="absolute inset-x-0 top-0 bg-widget" style={{ height: TITLE_HEIGHT }} />
      <div className="absolute inset-x-0" style={{ top: TITLE_HEIGHT, height: CONTENT_HEIGHT }}>
        <div className="absolute" style={{ left: knobLeft, top: knobTop, width: KNOB_AREA.width }}>
          <div className="flex items-center justify-center font-bold" style={{ height: LABEL_HEIGHT, fontSize: juceHeight(16) }}>
            GAIN
          </div>
          <Rotary backend={backend} id="masterGain" width={KNOB_AREA.width} height={KNOB_AREA.height - LABEL_HEIGHT} textBox={{ width: 80, height: 22 }} />
        </div>
      </div>
      <LevelMeter backend={backend} x={METER.left} y={METER.top} width={METER.width} height={METER.height} />
      <HoverTip text={`v${backend.version}`} target={TITLE} tip={TIP}>
        <div className="flex h-full items-center font-bold" style={{ fontSize: juceHeight(18) }}>
          JUCE STARTER
        </div>
      </HoverTip>
      <PresetBar presets={backend.presets} prompt={prompt} bounds={PRESET_BAR} />
      {overlay}
    </div>
  );
});
