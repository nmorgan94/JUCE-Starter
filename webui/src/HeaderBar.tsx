// Mirrors Source/ui/HeaderBar.cpp
import { useCallback, useRef, useState } from 'react';
import { aboutText } from './about.ts';
import type { PluginBackend } from './backend.ts';
import { TextButton } from './controls.tsx';
import { juceHeight } from './fonts.ts';
import { LevelMeter, PREFERRED_WIDTH } from './LevelMeter.tsx';
import { Menu, type MenuItem, type MenuState } from './Menu.tsx';
import { presetActions } from './presetActions.ts';
import { PresetBar } from './PresetBar.tsx';
import type { Prompt } from './PromptOverlay.tsx';

export const HEADER_HEIGHT = 44;

// HeaderBar::resized
const layOut = (width: number) => {
  const meter = { left: width - 12 - PREFERRED_WIDTH, top: 4, width: PREFERRED_WIDTH, height: HEADER_HEIGHT - 8 };
  const controlsLeft = 152;
  const controls = { left: controlsLeft, top: 10, width: meter.left - 12 - controlsLeft, height: HEADER_HEIGHT - 20 };
  const menu = { ...controls, left: controls.left + controls.width - 28, width: 28 };
  return {
    title: { left: 12, top: 0, height: HEADER_HEIGHT },
    meter,
    menu,
    presetBar: { ...controls, width: controls.width - 28 - 4 },
  };
};

interface HeaderBarProps {
  title: string;
  width: number;
  backend: PluginBackend;
  prompt: Prompt;
  showManual(): void;
}

// A fragment, not a box, so the menus stay positioned against the editor
export function HeaderBar({ title, width, backend, prompt, showManual }: HeaderBarProps) {
  const [menu, setMenu] = useState<MenuState | null>(null);
  const menuButton = useRef<HTMLButtonElement>(null);
  const closeMenu = useCallback(() => setMenu(null), []);
  const layout = layOut(width);
  const actions = presetActions(backend.presets, prompt);

  const showMenu = async () => {
    if (menu) return setMenu(null);

    const items: MenuItem[] = [
      ...(await actions.menuItems()),
      { kind: 'separator' },
      { kind: 'item', label: 'Manual', onSelect: showManual },
      { kind: 'item', label: 'About', onSelect: () => prompt.showMessage('About', aboutText(backend.about)) },
    ];
    setMenu({ items, target: layout.menu, owner: menuButton.current });
  };

  return (
    <>
      <div className="absolute inset-x-0 top-0 bg-widget" style={{ height: HEADER_HEIGHT }} />
      <LevelMeter backend={backend} x={layout.meter.left} y={layout.meter.top} width={layout.meter.width} height={layout.meter.height} />
      <div className="absolute flex items-center font-bold" style={{ ...layout.title, fontSize: juceHeight(18) }}>
        {title}
      </div>
      <PresetBar presets={backend.presets} actions={actions} bounds={layout.presetBar} />
      <TextButton ref={menuButton} text="..." style={layout.menu} onClick={() => void showMenu()} />
      {menu && <Menu {...menu} onClose={closeMenu} />}
    </>
  );
}
