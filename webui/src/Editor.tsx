// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo } from 'react';
import type { PluginBackend } from './backend.ts';
import { Rotary } from './controls.tsx';
import { juceHeight } from './fonts.ts';
import { LevelMeter, PREFERRED_WIDTH } from './LevelMeter.tsx';
import { HoverTip } from './HoverTip.tsx';

export const EDITOR_WIDTH = 400;
export const EDITOR_HEIGHT = 300;

const TITLE_HEIGHT = 32;
const TITLE = { left: 12, top: 0, height: TITLE_HEIGHT };
const TIP = { left: TITLE.left, top: TITLE_HEIGHT + 4, height: 22 };
const CONTENT_HEIGHT = EDITOR_HEIGHT - TITLE_HEIGHT;

const METER_STRIP = 50;
const METER_HEIGHT = 160;

const KNOB_AREA = { width: 140, height: 190 };
const LABEL_HEIGHT = 24;

// juce::Rectangle::withSizeKeepingCentre
const centred = (outer: number, inner: number) => Math.trunc((outer - inner) / 2);

const knobLeft = centred(EDITOR_WIDTH - METER_STRIP, KNOB_AREA.width);
const knobTop = centred(CONTENT_HEIGHT, KNOB_AREA.height);

export const Editor = memo(function Editor({ backend }: { backend: PluginBackend }) {
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
        <LevelMeter
          backend={backend}
          x={EDITOR_WIDTH - METER_STRIP + centred(METER_STRIP, PREFERRED_WIDTH)}
          y={centred(CONTENT_HEIGHT, METER_HEIGHT)}
          width={PREFERRED_WIDTH}
          height={METER_HEIGHT}
        />
      </div>
      <HoverTip text={`v${backend.version}`} target={TITLE} tip={TIP}>
        <div className="flex h-full items-center font-bold" style={{ fontSize: juceHeight(18) }}>
          JUCE STARTER
        </div>
      </HoverTip>
    </div>
  );
});
