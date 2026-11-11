import assert from 'node:assert/strict';
import { test } from 'node:test';
import { blockSize, maxError, sine } from '@samplerealm/modules/testing/engine.ts';
import { engine } from './engine.ts';

test('the ui info carries the version, the commit, the build type and every parameter as ParameterSpecs defines it', () => {
  const { info } = engine();

  assert.match(info.version, /^\d+\.\d+\.\d+$/);
  assert.match(info.commit, /^([0-9a-f]{7,}(-local)?|unknown)$/);
  assert.match(info.build, /^(Debug|Release)$/);
  assert.deepEqual(info.params, [
    { id: 'masterGain', name: 'Master Gain', kind: 'real', min: -60, max: 6, defaultValue: 0, step: 0.1, skew: 1, unit: 'dB', choices: [] },
  ]);
});

test('a stereo sine comes out with the gain applied to both channels', () => {
  for (const [gainDb, gain] of [[0, 1], [-6, 0.501187], [6, 1.995262], [-60, 0]]) {
    const stereo = engine();
    stereo.set('masterGain', gainDb);
    const input = [sine(0.1, 0.5), sine(0.1, 0.25, 220)];

    const output = stereo.processBlocks(input);

    output.forEach((channel, channelIndex) => {
      assert.ok(channel.every(Number.isFinite));
      assert.ok(maxError(channel, input[channelIndex].map((sample) => sample * gain)) < 1e-4, `${gainDb} dB, channel ${channelIndex}`);
    });
  }
});

test('a mono layout processes only the first channel', () => {
  const mono = engine(1);
  mono.set('masterGain', -6);
  mono.view(0, blockSize).fill(0.5);
  mono.view(1, blockSize).fill(0.5);
  mono.engine.render(blockSize);

  assert.ok(Math.abs(mono.view(0, blockSize)[0] - 0.5 * 0.501187) < 1e-4);
  assert.equal(mono.view(1, blockSize)[0], 0.5);
  assert.equal(mono.engine.peak(1), 0);
});

test('the peak is the loudest output since the last read, after the gain', () => {
  const stereo = engine();
  stereo.set('masterGain', -6);
  stereo.processBlocks([sine(0.05, 0.8), sine(0.05, 0.4)]);

  assert.ok(Math.abs(stereo.engine.peak(0) - 0.8 * 0.501187) < 1e-3, 'channel 0');
  assert.ok(Math.abs(stereo.engine.peak(1) - 0.4 * 0.501187) < 1e-3, 'channel 1');
  assert.equal(stereo.engine.peak(0), 0);
});
