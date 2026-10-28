// The C++ side of these names is Source/webview/WebViewEditor.cpp
import { getSliderState } from '@juce-framework/webview';
import { paramLookup, type Level, type PluginBackend, type UiInfo } from '../backend.ts';
import { toProportion } from '../params.ts';

// JUCE's type lists only its own initialisation data
type InitialisationData = { uiInfo: string[] };

export function juceBackend(): PluginBackend {
  const { uiInfo } = window.__JUCE__.initialisationData as unknown as InitialisationData;
  const info = JSON.parse(uiInfo[0]) as UiInfo;
  const entry = paramLookup(new Map(info.params.map((param) => [param.id, { param, slider: getSliderState(param.id) }])));

  const param = (id: string) => entry(id).param;
  const slider = (id: string) => entry(id).slider;

  return {
    param,
    getValue: (id) => slider(id).getScaledValue(),
    setValue: (id, value) => slider(id).setNormalisedValue(toProportion(param(id), value)),
    beginGesture: (id) => slider(id).sliderDragStarted(),
    endGesture: (id) => slider(id).sliderDragEnded(),
    subscribe: (id, listener) => {
      const state = slider(id);
      const handle = state.valueChangedEvent.addListener(() => listener(state.getScaledValue()));
      return () => state.valueChangedEvent.removeListener(handle);
    },
    onLevel: (listener) => {
      const handle = window.__JUCE__.backend.addEventListener('level', (payload) => listener(payload as Level));
      return () => window.__JUCE__.backend.removeEventListener(handle);
    },
  };
}
