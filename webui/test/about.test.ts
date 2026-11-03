import assert from 'node:assert/strict';
import { test } from 'node:test';
import { aboutText } from '../src/about.ts';

test('about shows the version and commit, where it runs, and the UI', () => {
  assert.equal(
    aboutText({ version: '1.2.3', commit: 'abc1234-local', running: 'VST3 in Ableton Live' }),
    'Version 1.2.3\nCommit abc1234-local\nVST3 in Ableton Live\nWebView UI',
  );
});
