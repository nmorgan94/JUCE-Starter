// Mirrors Source/ui/PresetBar.cpp
import { useCallback, useRef, useState, useSyncExternalStore } from 'react';
import { isSamePreset, type PresetBackend, type PresetEntry } from './backend.ts';
import { TextButton } from './controls.tsx';
import type { Box } from './layout.ts';
import { Menu, type MenuItem, type MenuState } from './Menu.tsx';
import type { PresetActions } from './presetActions.ts';

// PresetBar::resized
function layOut({ left, top, width, height }: Box) {
  const at = (x: number, w: number): Box => ({ left: x, top, width: w, height });
  const save = at(left + width - 48, 48);
  const previous = at(left, 24);
  const next = at(save.left - 8 - 24, 24);
  const nameLeft = previous.left + previous.width + 4;
  return { previous, next, save, name: at(nameLeft, next.left - 4 - nameLeft) };
}

interface PresetBarProps {
  presets: PresetBackend;
  actions: PresetActions;
  bounds: Box;
}

export function PresetBar({ presets, actions, bounds }: PresetBarProps) {
  const status = useSyncExternalStore(presets.onStatus, presets.status);
  const [menu, setMenu] = useState<MenuState | null>(null);
  const nameButton = useRef<HTMLButtonElement>(null);
  const layout = layOut(bounds);
  const closeMenu = useCallback(() => setMenu(null), []);

  const showPresetMenu = async () => {
    if (menu) return setMenu(null);

    const list = await presets.list();
    const { current } = presets.status();
    const item = (preset: PresetEntry): MenuItem => ({
      kind: 'item',
      label: preset.name,
      ticked: isSamePreset(preset, current),
      onSelect: () => void presets.load(preset),
    });

    // Factory presets by category, user presets flat
    const categories = new Map<string, PresetEntry[]>();
    for (const preset of list.filter((p) => p.isFactory)) {
      const category = preset.category || 'Other';
      categories.set(category, [...(categories.get(category) ?? []), preset]);
    }
    const users = list.filter((preset) => !preset.isFactory);

    const items: MenuItem[] = [{ kind: 'header', label: 'Factory' }];
    for (const category of [...categories.keys()].sort()) {
      const entries = categories.get(category)!;
      items.push({ kind: 'submenu', label: category, ticked: entries.some((p) => isSamePreset(p, current)), items: entries.map(item) });
    }
    if (categories.size === 0) items.push({ kind: 'item', label: 'No factory presets yet', disabled: true });

    items.push({ kind: 'header', label: 'User' }, ...users.map(item));
    if (users.length === 0) items.push({ kind: 'item', label: 'No user presets yet', disabled: true });

    setMenu({ items, target: layout.name, owner: nameButton.current });
  };

  return (
    <>
      <TextButton text="<" style={layout.previous} onClick={() => void presets.loadPrevious()} />
      <TextButton ref={nameButton} text={status.current.name + (status.modified ? ' *' : '')} style={layout.name} onClick={() => void showPresetMenu()} />
      <TextButton text=">" style={layout.next} onClick={() => void presets.loadNext()} />
      <TextButton text="Save" style={layout.save} onClick={() => void actions.save()} />
      {menu && <Menu {...menu} onClose={closeMenu} />}
    </>
  );
}
