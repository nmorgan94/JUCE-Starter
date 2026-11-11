// This plugin's factory preset files in the demo
import assert from 'node:assert/strict';
import { readdirSync, readFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { test } from 'node:test';
import { workletBackend } from '@samplerealm/modules/demo/workletBackend.ts';
import { info } from '../engine.ts';

const presetFolder = resolve(import.meta.dirname, '../../../Assets/Presets');
const factoryPresets = readdirSync(presetFolder)
  .filter((file) => file.endsWith('.srstarter'))
  .map((file) => readFileSync(resolve(presetFolder, file), 'utf8'));

// The same order as Tests/PresetTests.cpp
const FACTORY_NAMES = ['Boost +3', 'Boost +6', 'Cut -6', 'Cut -12', 'Cut -24'];

test('factory presets load into the engine in the plugin\'s order', async () => {
  const { presets, getValue, param } = workletBackend(info, { storageKey: 'test-presets', factoryPresets });
  const list = await presets.list();
  assert.deepEqual(list.map((preset) => preset.name), FACTORY_NAMES);

  await presets.load(list.find((preset) => preset.name === 'Cut -12')!);
  assert.equal(getValue('masterGain'), -12);

  await presets.loadInit();
  assert.equal(getValue('masterGain'), param('masterGain').defaultValue);
});
