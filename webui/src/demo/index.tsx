import { useCallback, useEffect, useRef, useState } from 'react';
import { createRoot } from 'react-dom/client';
import { EDITOR_HEIGHT, EDITOR_WIDTH, Editor } from '../Editor.tsx';
import { juceHeight } from '../fonts.ts';
import { factoryPresets } from './factoryPresets.ts';
import { makeLoop } from './loop.ts';
import { loadEngine, workletBackend, type WorkletBackend } from './workletBackend.ts';
import './styles.css';

const ENGINE_URL = 'engine.wasm';
const PROCESSOR_URL = 'processor.js';

// As the plugin's default stereo layout
const CHANNELS = 2;

const HEIGHT_MESSAGE = 'samplerealm-demo-height';

// Blocked storage throws on access
function browserStorage() {
  try {
    return window.localStorage;
  } catch {
    return undefined;
  }
}

interface Audio {
  context: AudioContext;
  node: AudioWorkletNode;
  loop: AudioBuffer;
}

async function startAudio(backend: WorkletBackend, wasm: ArrayBuffer, context: AudioContext): Promise<Audio> {
  await context.audioWorklet.addModule(PROCESSOR_URL);
  const loop = makeLoop(context);

  const node = new AudioWorkletNode(context, 'engine-processor', {
    numberOfInputs: 1,
    outputChannelCount: [CHANNELS],
    channelCount: CHANNELS,
    channelCountMode: 'explicit',
    processorOptions: { wasm, channels: CHANNELS },
  });

  await new Promise<void>((resolve, reject) => {
    node.port.onmessage = ({ data }) => {
      if (data.type === 'ready') resolve();
      else if (data.type === 'error') reject(new Error(data.message));
    };
  });

  node.connect(context.destination);
  backend.attach(node.port);
  return { context, node, loop };
}

function useScale() {
  const [scale, setScale] = useState(1);
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const element = ref.current;
    if (!element) return;
    const observer = new ResizeObserver(() => setScale(Math.min(1, element.clientWidth / EDITOR_WIDTH)));
    observer.observe(element);
    return () => observer.disconnect();
  }, []);

  return { ref, scale };
}

// The loop runs through the engine, so Pause also lets the meter fall
function usePlayer(audio: Audio | null) {
  const [playing, setPlaying] = useState(false);
  const source = useRef<AudioBufferSourceNode | null>(null);
  const offset = useRef(0);
  const startedAt = useRef(0);

  const play = useCallback(() => {
    if (!audio || source.current) return;
    const { context, node, loop } = audio;
    context.resume();
    const next = new AudioBufferSourceNode(context, { buffer: loop, loop: true });
    next.connect(node);
    next.start(0, offset.current);
    startedAt.current = context.currentTime - offset.current;
    source.current = next;
    setPlaying(true);
  }, [audio]);

  const pause = useCallback(() => {
    if (!audio || !source.current) return;
    offset.current = (audio.context.currentTime - startedAt.current) % audio.loop.duration;
    source.current.stop();
    source.current.disconnect();
    source.current = null;
    audio.context.suspend();
    setPlaying(false);
  }, [audio]);

  return { playing, toggle: playing ? pause : play };
}

function Demo() {
  const [engine, setEngine] = useState<{ backend: WorkletBackend; wasm: ArrayBuffer } | null>(null);
  const [audio, setAudio] = useState<Audio | null>(null);
  const [failed, setFailed] = useState(false);
  const context = useRef<AudioContext | null>(null);
  const { ref, scale } = useScale();
  const player = usePlayer(audio);

  useEffect(() => {
    loadEngine(ENGINE_URL)
      .then(({ wasm, info }) => setEngine({ backend: workletBackend(info, factoryPresets, browserStorage()), wasm }))
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
    <div className="mx-auto max-w-100" ref={ref}>
      <div className="relative overflow-hidden rounded-xl" style={{ width: EDITOR_WIDTH * scale, height: EDITOR_HEIGHT * scale }}>
        <div className="origin-top-left" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT, transform: `scale(${scale})` }}>
          {engine && <Editor backend={engine.backend} />}
        </div>
        {(failed || !audio) && (
          <div className="absolute inset-0 z-30 flex items-center justify-center bg-background/55">
            {failed ? (
              <p className="text-sm text-clip">The demo couldn't start. Please try again later.</p>
            ) : (
              <p className="text-sm font-bold">Loading…</p>
            )}
          </div>
        )}
      </div>

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
    </div>
  );
}

function mount(element: Element) {
  createRoot(element).render(<Demo />);

  // Not the document's height, which never shrinks below the frame
  if (window.parent !== window) {
    const report = () => window.parent.postMessage({ type: HEIGHT_MESSAGE, height: Math.ceil(element.getBoundingClientRect().height) }, '*');
    new ResizeObserver(report).observe(element);
  }
}

const root = document.getElementById('root');
if (root) mount(root);
