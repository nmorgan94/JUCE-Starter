import assert from 'node:assert/strict';
import { readdirSync, readFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { test } from 'node:test';
import { parsePreset, STORAGE_KEY, type PresetStorage } from '../../src/demo/localPresets.ts';
import { workletBackend } from '../../src/demo/workletBackend.ts';
import { info } from '../engine.ts';

const presetFolder = resolve(import.meta.dirname, '../../../Assets/Presets');
const factory = readdirSync(presetFolder)
  .filter((file) => file.endsWith('.srstarter'))
  .map((file) => parsePreset(readFileSync(resolve(presetFolder, file), 'utf8')));

// The same order as Tests/PresetTests.cpp
const FACTORY_NAMES = ['Boost +3', 'Boost +6', 'Cut -6', 'Cut -12', 'Cut -24'];

const fakeStorage = (): PresetStorage & { items: Map<string, string> } => {
  const items = new Map<string, string>();
  return { items, getItem: (key) => items.get(key) ?? null, setItem: (key, value) => void items.set(key, value) };
};

const setUp = (storage = fakeStorage()) => {
  const backend = workletBackend(info, factory, storage);
  return { backend, presets: backend.presets, storage, gain: () => backend.getValue('masterGain') };
};

const named = <T extends { name: string }>(list: T[], name: string) => {
  const found = list.find((preset) => preset.name === name);
  assert.ok(found, `No preset ${name}`);
  return found;
};

test('factory files are read with their category and values', () => {
  const cut = named(factory, 'Cut -12');
  assert.deepEqual(cut, { name: 'Cut -12', category: 'Cut', isFactory: true, values: { masterGain: -12 } });
});

test('factory presets are listed by category, then name', async () => {
  const { presets } = setUp();
  assert.deepEqual((await presets.list()).map((preset) => preset.name), FACTORY_NAMES);
});

test('a loaded preset sets its values and shows its name', async () => {
  const { presets, gain } = setUp();
  assert.equal(presets.status().current.name, 'Init');

  await presets.load(named(await presets.list(), 'Cut -12'));

  assert.equal(gain(), -12);
  assert.deepEqual(presets.status(), { current: { name: 'Cut -12', category: 'Cut', isFactory: true }, modified: false });
});

test('moving a parameter marks the preset modified until the next load', async () => {
  const { backend, presets, gain } = setUp();
  const boost = named(await presets.list(), 'Boost +3');
  await presets.load(boost);

  const seen: boolean[] = [];
  presets.onStatus((status) => seen.push(status.modified));

  backend.setValue('masterGain', -1);
  assert.equal(presets.status().modified, true);

  await presets.load(boost);
  assert.equal(presets.status().modified, false);
  assert.equal(gain(), 3);
  assert.deepEqual(seen, [true, false]);
});

test('Init restores the defaults', async () => {
  const { backend, presets, gain } = setUp();
  await presets.load(named(await presets.list(), 'Boost +6'));
  backend.setValue('masterGain', 4);

  await presets.loadInit();

  assert.equal(gain(), backend.param('masterGain').defaultValue);
  assert.equal(presets.status().current.name, 'Init');
  assert.equal(presets.status().modified, false);
});

test('next and previous wrap, and start from either end after Init', async () => {
  const { presets, gain } = setUp();
  const name = () => presets.status().current.name;

  await presets.loadNext();
  assert.equal(name(), FACTORY_NAMES[0]);

  await presets.loadPrevious();
  assert.equal(name(), FACTORY_NAMES.at(-1));

  await presets.loadNext();
  assert.equal(name(), FACTORY_NAMES[0]);

  await presets.loadInit();
  await presets.loadPrevious();
  assert.equal(name(), FACTORY_NAMES.at(-1));
  assert.equal(gain(), -24);
});

test('user presets save, rename and delete, and outlast the page', async () => {
  const { backend, presets, storage } = setUp();
  backend.setValue('masterGain', -3);

  assert.equal(await presets.save('Mine', 'Cut'), null);
  assert.deepEqual(presets.status(), { current: { name: 'Mine', category: 'Cut', isFactory: false }, modified: false });
  assert.equal(await presets.exists('mine'), true);
  assert.deepEqual((await presets.list()).map((preset) => preset.name), [...FACTORY_NAMES, 'Mine']);

  // A new page reads it back
  const reopened = setUp(storage);
  await reopened.presets.load(named(await reopened.presets.list(), 'Mine'));
  assert.equal(reopened.gain(), -3);

  assert.equal(await presets.rename(named(await presets.list(), 'Mine'), 'Yours'), null);
  assert.equal(presets.status().current.name, 'Yours');
  assert.deepEqual((await presets.list()).filter((preset) => !preset.isFactory).map((preset) => preset.name), ['Yours']);

  assert.equal(await presets.remove(named(await presets.list(), 'Yours')), null);
  assert.equal(await presets.exists('Yours'), false);
  assert.equal(presets.status().modified, true);
  assert.equal(storage.items.get(STORAGE_KEY), '[]');
});

test('user presets can\'t take a factory name', async () => {
  const { presets } = setUp();

  assert.match((await presets.save('boost +3', '')) ?? '', /factory preset/);
  assert.equal(await presets.save('   ', ''), 'Enter a preset name.');
  assert.equal(await presets.exists('boost +3'), false);

  assert.equal(await presets.save('Mine', ''), null);
  assert.match((await presets.rename(named(await presets.list(), 'Mine'), 'Cut -6')) ?? '', /factory preset/);
  assert.equal(await presets.exists('Mine'), true);
});

test('a stored preset loads past ids this engine doesn\'t have', async () => {
  const storage = fakeStorage();
  storage.setItem(STORAGE_KEY, JSON.stringify([{ name: 'Other', category: '', values: { cutoff: 1, masterGain: -3 } }]));
  const { presets, gain } = setUp(storage);

  await presets.load(named(await presets.list(), 'Other'));

  assert.equal(gain(), -3);
  assert.equal(presets.status().current.name, 'Other');
});

test('next and previous skip a user preset deleted in another tab', async () => {
  const { presets, storage } = setUp();
  await presets.save('A', '');
  await presets.save('B', '');
  await presets.load(named(await presets.list(), 'A'));

  const otherTab = setUp(storage).presets;
  assert.equal(await otherTab.remove(named(await otherTab.list(), 'B')), null);

  await presets.loadNext();
  assert.equal(presets.status().current.name, FACTORY_NAMES[0]);
});

test('unreadable storage leaves only the factory presets', async () => {
  const storage = fakeStorage();
  storage.setItem(STORAGE_KEY, 'not json');
  const { presets } = setUp(storage);
  assert.deepEqual((await presets.list()).map((preset) => preset.name), FACTORY_NAMES);
});
