// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo, useState } from 'react';
import { ActivationBar } from '@samplerealm/modules/editor/ActivationBar.tsx';
import { EditorRootContext } from '@samplerealm/modules/editor/editorRoot.ts';
import { HeaderBar, type HeaderLayout } from '@samplerealm/modules/editor/HeaderBar.tsx';
import { LabeledSlider } from '@samplerealm/modules/editor/LabeledSlider.tsx';
import { centred } from '@samplerealm/modules/editor/layout.ts';
import { LevelMeter, PREFERRED_WIDTH } from '@samplerealm/modules/editor/LevelMeter.tsx';
import { useManual } from '@samplerealm/modules/editor/ManualOverlay.tsx';
import { usePrompt } from '@samplerealm/modules/editor/PromptOverlay.tsx';
import manualText from '../../Assets/Manual.md?raw';
import type { PluginBackend } from './backend.ts';

export const EDITOR_WIDTH = 440;
export const EDITOR_HEIGHT = 312;

// AudioPluginAudioProcessorEditor::headerLayout
const HEADER_LAYOUT: HeaderLayout = {
  height: 44,
  edge: 12,
  titleWidth: 128,
  titleHeight: 18,
  gap: 12,
  controlsHeight: 24,
  meterWidth: PREFERRED_WIDTH,
  meterInset: 4,
};

const CONTENT_HEIGHT = EDITOR_HEIGHT - HEADER_LAYOUT.height;

const KNOB_AREA = { width: 140, height: 190 };

const knobLeft = centred(EDITOR_WIDTH, KNOB_AREA.width);
const knobTop = centred(CONTENT_HEIGHT, KNOB_AREA.height);

export const Editor = memo(function Editor({ backend }: { backend: PluginBackend }) {
  const [root, setRoot] = useState<HTMLDivElement | null>(null);
  const { prompt, overlay } = usePrompt();
  const manual = useManual(manualText);

  return (
    <EditorRootContext.Provider value={root}>
      <div ref={setRoot} className="relative bg-background" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT }}>
        <div className="absolute inset-x-0" style={{ top: HEADER_LAYOUT.height, height: CONTENT_HEIGHT }}>
          <div className="absolute" style={{ left: knobLeft, top: knobTop }}>
            <LabeledSlider backend={backend} id="masterGain" label="GAIN" width={KNOB_AREA.width} height={KNOB_AREA.height} />
          </div>
        </div>
        <HeaderBar
          title="JUCE STARTER"
          width={EDITOR_WIDTH}
          layout={HEADER_LAYOUT}
          backend={backend}
          prompt={prompt}
          showManual={manual.showManual}
          meter={({ left, top, width, height }) => <LevelMeter backend={backend} x={left} y={top} width={width} height={height} />}
        />
        {backend.activation && <ActivationBar activation={backend.activation} width={EDITOR_WIDTH} prompt={prompt} />}
        {overlay}
        {manual.overlay}
      </div>
    </EditorRootContext.Provider>
  );
});
