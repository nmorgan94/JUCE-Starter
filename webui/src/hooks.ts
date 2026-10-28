import { useCallback, useEffect, useState } from 'react';
import type { PluginBackend } from './backend.ts';

export function useParam(backend: PluginBackend, id: string) {
  const [value, setLocal] = useState(() => backend.getValue(id));

  useEffect(() => {
    setLocal(backend.getValue(id));
    return backend.subscribe(id, setLocal);
  }, [backend, id]);

  const set = useCallback(
    (next: number) => {
      setLocal(next);
      backend.setValue(id, next);
    },
    [backend, id]
  );

  const commit = useCallback(
    (next: number) => {
      backend.beginGesture(id);
      set(next);
      backend.endGesture(id);
    },
    [backend, id, set]
  );

  return { value, set, commit, param: backend.param(id) };
}
