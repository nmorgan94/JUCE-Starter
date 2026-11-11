import { engineAt } from '@samplerealm/modules/testing/engine.ts';
import { localEnginePath } from '../localEngine.ts';
import { paramLookup } from '../src/backend.ts';

// Makes a fresh engine from the last wasm build
export const engine = engineAt(localEnginePath());

export const { info } = engine();

export const param = paramLookup(new Map(info.params.map((p) => [p.id, p])));
