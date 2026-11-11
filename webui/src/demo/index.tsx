import { useCallback, useEffect, useRef, useState } from 'react';
import { createRoot } from 'react-dom/client';
import { DemoFrame } from '@samplerealm/modules/demo/DemoFrame.tsx';
import { loadEngine, startWorklet } from '@samplerealm/modules/demo/engine.ts';
import processorUrl from '@samplerealm/modules/demo/processor.js?url';
import { workletBackend, type WorkletBackend } from '@samplerealm/modules/demo/workletBackend.ts';
import { EDITOR_HEIGHT, EDITOR_WIDTH, Editor } from '../Editor.tsx';
import { juceHeight } from '../fonts.ts';
import { factoryPresets } from './factoryPresets.ts';
import { loopTransport, makeLoop } from './loop.ts';
import './styles.css';

const ENGINE_URL = 'engine.wasm';

// This plugin's, as a site's demos share its storage
const STORAGE_KEY = 'juce-starter-presets';

// As the plugin's default stereo layout
const CHANNELS = 2;

interface Audio {
  context: AudioContext;
  node: AudioWorkletNode;
  loop: AudioBuffer;
}

async function startAudio(backend: WorkletBackend, wasm: ArrayBuffer, context: AudioContext): Promise<Audio> {
  const node = await startWorklet(context, processorUrl, {
    numberOfInputs: 1,
    outputChannelCount: [CHANNELS],
    channelCount: CHANNELS,
    channelCountMode: 'explicit',
    processorOptions: { wasm, channels: CHANNELS },
  });

  node.connect(context.destination);
  backend.attach(node.port);
  return { context, node, loop: makeLoop(context) };
}

// The loop runs through the engine, so Pause also lets the meter fall
function usePlayer(audio: Audio | null, backend: WorkletBackend | undefined) {
  const [playing, setPlaying] = useState(false);
  const source = useRef<AudioBufferSourceNode | null>(null);
  // Seconds played, counting every time round the loop, as a host's position does
  const offset = useRef(0);
  const startedAt = useRef(0);

  const play = useCallback(() => {
    if (!audio || !backend || source.current) return;
    const { context, node, loop } = audio;
    // Before resuming, so the engine has it for the first block
    backend.setTransport(loopTransport(true, offset.current));
    context.resume();
    const next = new AudioBufferSourceNode(context, { buffer: loop, loop: true });
    next.connect(node);
    next.start(0, offset.current % loop.duration);
    startedAt.current = context.currentTime - offset.current;
    source.current = next;
    setPlaying(true);
  }, [audio, backend]);

  const pause = useCallback(() => {
    if (!audio || !backend || !source.current) return;
    offset.current = audio.context.currentTime - startedAt.current;
    source.current.stop();
    source.current.disconnect();
    source.current = null;
    audio.context.suspend();
    backend.setTransport(loopTransport(false, offset.current));
    setPlaying(false);
  }, [audio, backend]);

  return { playing, toggle: playing ? pause : play };
}

function Demo() {
  const [engine, setEngine] = useState<{ backend: WorkletBackend; wasm: ArrayBuffer } | null>(null);
  const [audio, setAudio] = useState<Audio | null>(null);
  const [failed, setFailed] = useState(false);
  const context = useRef<AudioContext | null>(null);
  const player = usePlayer(audio, engine?.backend);

  useEffect(() => {
    loadEngine(ENGINE_URL)
      .then(({ wasm, info }) => setEngine({ backend: workletBackend(info, { storageKey: STORAGE_KEY, factoryPresets }), wasm }))
      .catch((error) => {
        console.error('Demo failed to load:', error);
        setFailed(true);
      });
  }, []);

  // Ready to play at once; the loop waits for Play
  useEffect(() => {
    if (!engine || context.current) return;

    const audioContext = new AudioContext();
    // It starts running where autoplay is allowed
    audioContext.suspend();
    context.current = audioContext;

    startAudio(engine.backend, engine.wasm, audioContext)
      .then(setAudio)
      .catch((error) => {
        console.error('Demo audio failed to start:', error);
        audioContext.close();
        setFailed(true);
      });
  }, [engine]);

  useEffect(() => () => void context.current?.close(), []);

  return (
    <DemoFrame
      width={EDITOR_WIDTH}
      height={EDITOR_HEIGHT}
      status={failed ? 'failed' : audio ? 'ready' : 'loading'}
      editor={engine && <Editor backend={engine.backend} />}
    >
      <button
        type="button"
        className="mt-2.5 h-7 w-20 cursor-pointer border border-line bg-background text-fg disabled:cursor-default disabled:opacity-50"
        style={{ fontSize: juceHeight(15) }}
        disabled={!audio}
        aria-pressed={player.playing}
        onClick={player.toggle}
      >
        {player.playing ? 'PAUSE' : 'PLAY'}
      </button>
    </DemoFrame>
  );
}

const root = document.getElementById('root');
if (root) createRoot(root).render(<Demo />);
