import assert from 'node:assert/strict';
import { test } from 'node:test';
import { formatValue, fromProportion } from '@samplerealm/modules/editor/params.ts';
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
