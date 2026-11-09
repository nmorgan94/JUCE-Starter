import assert from 'node:assert/strict';
import { test } from 'node:test';
import { workletBackend, type EnginePort } from '../../src/demo/workletBackend.ts';
import { info } from '../engine.ts';

const fakePort = () => {
  const posted: unknown[] = [];
  const port: EnginePort = { postMessage: (message: unknown) => void posted.push(message), onmessage: null };
  return { port, posted };
};

test('changes made before the worklet attaches reach it once it does, in order and once each', () => {
  const backend = workletBackend(info);
  backend.setValue('masterGain', -12);
  backend.setValue('masterGain', -3);

  const { port, posted } = fakePort();
  backend.attach(port);
  backend.setValue('masterGain', 2);

  assert.deepEqual(posted, [
    { type: 'param', index: 0, value: -12 },
    { type: 'param', index: 0, value: -3 },
    { type: 'param', index: 0, value: 2 },
  ]);
});

test('values read back and notify before the worklet attaches', () => {
  const backend = workletBackend(info);
  const heard: number[] = [];
  backend.subscribe('masterGain', (value) => heard.push(value));

  assert.equal(backend.getValue('masterGain'), 0);
  backend.setValue('masterGain', -6);

  assert.equal(backend.getValue('masterGain'), -6);
  assert.deepEqual(heard, [-6]);
});
