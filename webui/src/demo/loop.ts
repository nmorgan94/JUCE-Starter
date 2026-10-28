// A placeholder for the demo's input: four bars at 120 BPM
const BPM = 120;
const BEATS = 16;
const PEAK = 0.7;

const midiToHz = (note: number) => 440 * 2 ** ((note - 69) / 12);

export function makeLoop(context: BaseAudioContext): AudioBuffer {
  const rate = context.sampleRate;
  const beat = 60 / BPM;
  const length = Math.round(BEATS * beat * rate);
  const left = new Float32Array(length);
  const right = new Float32Array(length);

  let seed = 1;
  const noise = () => ((seed = (seed * 1664525 + 1013904223) >>> 0) / 2 ** 32) * 2 - 1;

  const add = (start: number, seconds: number, pan: number, voice: (t: number) => number) => {
    const from = Math.round(start * rate);
    for (let i = 0; i < seconds * rate; i++) {
      const sample = voice(i / rate);
      // Tails wrap, so the loop has no seam
      const index = (from + i) % length;
      left[index] += sample * (1 - pan);
      right[index] += sample * (1 + pan);
    }
  };

  for (let b = 0; b < BEATS; b++) {
    add(b * beat, 0.35, 0, (t) => 0.8 * Math.sin(2 * Math.PI * (45 * t + (60 * (1 - Math.exp(-t * 30))) / 30)) * Math.exp(-t * 9));
  }

  let last = 0;
  for (let offBeat = 1; offBeat < BEATS * 2; offBeat += 2) {
    add((offBeat * beat) / 2, 0.06, 0.3, (t) => {
      const n = noise();
      const high = n - last;
      last = n;
      return 0.18 * high * Math.exp(-t * 70);
    });
  }

  const bass = [45, 45, 48, 43];
  const chords = [[57, 60, 64], [57, 60, 64], [60, 64, 67], [55, 59, 62]];
  for (let bar = 0; bar < 4; bar++) {
    const f = midiToHz(bass[bar]);
    for (const step of [0, 1.5, 2.5]) {
      add((bar * 4 + step) * beat, beat * 0.9, -0.1, (t) => 0.3 * Math.tanh(2 * Math.sin(2 * Math.PI * f * t)) * Math.exp(-t * 4));
    }

    for (const step of [1, 3]) {
      chords[bar].forEach((note, voice) => {
        const f = midiToHz(note);
        add((bar * 4 + step) * beat, 0.5, voice - 1, (t) => 0.07 * (Math.sin(2 * Math.PI * f * t) + 0.3 * Math.sin(4 * Math.PI * f * t)) * Math.exp(-t * 7));
      });
    }
  }

  const peak = [left, right].reduce((max, channel) => channel.reduce((m, s) => Math.max(m, Math.abs(s)), max), 0);
  const buffer = context.createBuffer(2, length, rate);
  [left, right].forEach((channel, i) => buffer.copyToChannel(channel.map((s) => (s * PEAK) / peak), i));
  return buffer;
}
