import assert from 'node:assert/strict';
import { test } from 'node:test';
import { formatValue, fromProportion, parseValue, toProportion } from '../src/params.ts';
import { info, param } from './engine.ts';

// From the Catch2 snapshot in Tests/ProcessorTests.cpp
const pluginText: [id: string, proportion: number, text: string][] = [
  ['masterGain', 0, '-60.0'],
  ['masterGain', 0.5, '-27.0'],
  ['masterGain', 1, '6.0'],
];

test('every parameter has a row in pluginText', () => {
  for (const { id } of info.params) assert.ok(pluginText.some(([row]) => row === id), id);
});

test('every value reads as the plugin shows it, at the same knob position', () => {
  for (const [id, proportion, text] of pluginText) {
    assert.equal(formatValue(param(id), fromProportion(param(id), proportion)), text, `${id} at ${proportion}`);
  }
});

test('knob positions map to values and back', () => {
  const gain = param('masterGain');

  assert.equal(fromProportion(gain, 0.13), -51.4);
  assert.ok(Math.abs(toProportion(gain, fromProportion(gain, 0.25)) - 0.25) < 0.001);
});

test('positions past either end clamp to the range', () => {
  const gain = param('masterGain');

  assert.equal(fromProportion(gain, 1.5), 6);
  assert.equal(fromProportion(gain, -0.5), -60);
});

test('typed text parses with its unit, and text without a number is ignored', () => {
  const gain = param('masterGain');

  assert.equal(parseValue(gain, '-12 dB'), -12);
  assert.equal(parseValue(gain, '3.14'), 3.1);
  assert.equal(parseValue(gain, '99'), 6);
  assert.equal(parseValue(gain, 'loud'), null);
});
