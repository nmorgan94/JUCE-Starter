// Mirrors Source/ui/About.cpp
import type { About } from './backend.ts';

export const aboutText = ({ version, commit, running }: About) => `Version ${version}\nCommit ${commit}\n${running}\nWebView UI`;
