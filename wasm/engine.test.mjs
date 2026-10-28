import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { test } from 'node:test';
import { localEnginePath } from '../webui/localEngine.ts';
import { readUiInfo, stubImports } from '../webui/src/demo/workletBackend.ts';

const sampleRate = 48000;
const blockSize = 128;

const wasm = new WebAssembly.Module(readFileSync(localEnginePath()));

function engine(numChannels = 2) {
  const { exports } = new WebAssembly.Instance(wasm, stubImports());
  exports._initialize?.();
  exports.prepare(sampleRate, numChannels);

  const ids = readUiInfo(exports).params.map((p) => p.id);
  const set = (id, value) => {
    assert.ok(ids.includes(id), `no parameter ${id}`);
    exports.setParam(ids.indexOf(id), value);
  };

  const view = (channel, length) => new Float32Array(exports.memory.buffer, exports.channel(channel), length);

  // As an AudioWorklet: one block at a time, written in and read back from the engine's channels
  const processBlocks = (inputs) => {
    const outputs = inputs.map((input) => new Float32Array(input.length));
    for (let start = 0; start < inputs[0].length; start += blockSize) {
      const length = Math.min(blockSize, inputs[0].length - start);
      inputs.forEach((input, channel) => view(channel, length).set(input.subarray(start, start + length)));
      exports.render(length);
      outputs.forEach((output, channel) => output.set(view(channel, length), start));
    }
    return outputs;
  };

  return { ...exports, set, view, processBlocks };
}

const sine = (seconds, amplitude, frequency = 440) =>
  Float32Array.from({ length: Math.round(seconds * sampleRate) }, (_, i) => amplitude * Math.sin((2 * Math.PI * frequency * i) / sampleRate));

const maxError = (actual, expected) => Math.max(...actual.map((sample, i) => Math.abs(sample - expected[i])));

test('the ui info carries the version and every parameter as ParameterSpecs defines it', () => {
  const info = readUiInfo(engine());

  assert.match(info.version, /^\d+\.\d+\.\d+$/);
  assert.deepEqual(info.params, [
    { id: 'masterGain', name: 'Master Gain', kind: 'real', min: -60, max: 6, defaultValue: 0, step: 0.1, skew: 1, unit: 'dB', choices: [] },
  ]);
});

test('a stereo sine comes out with the gain applied to both channels', () => {
  for (const [gainDb, gain] of [[0, 1], [-6, 0.501187], [6, 1.995262], [-60, 0]]) {
    const e = engine();
    e.set('masterGain', gainDb);
    const input = [sine(0.1, 0.5), sine(0.1, 0.25, 220)];

    const output = e.processBlocks(input);

    output.forEach((channel, i) => {
      assert.ok(channel.every(Number.isFinite));
      assert.ok(maxError(channel, input[i].map((s) => s * gain)) < 1e-4, `${gainDb} dB, channel ${i}`);
    });
  }
});

test('a mono layout processes only the first channel', () => {
  const e = engine(1);
  e.set('masterGain', -6);
  e.view(0, blockSize).fill(0.5);
  e.view(1, blockSize).fill(0.5);
  e.render(blockSize);

  assert.ok(Math.abs(e.view(0, blockSize)[0] - 0.5 * 0.501187) < 1e-4);
  assert.equal(e.view(1, blockSize)[0], 0.5);
  assert.equal(e.peak(1), 0);
});

test('the peak is the loudest output since the last read, after the gain', () => {
  const e = engine();
  e.set('masterGain', -6);
  e.processBlocks([sine(0.05, 0.8), sine(0.05, 0.4)]);

  assert.ok(Math.abs(e.peak(0) - 0.8 * 0.501187) < 1e-3, 'channel 0');
  assert.ok(Math.abs(e.peak(1) - 0.4 * 0.501187) < 1e-3, 'channel 1');
  assert.equal(e.peak(0), 0);
});
