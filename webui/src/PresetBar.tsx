// Mirrors Source/ui/PresetBar.cpp
import { useCallback, useRef, useState, useSyncExternalStore } from 'react';
import { isSamePreset, type PresetBackend, type PresetEntry } from './backend.ts';
import { TextButton } from './controls.tsx';
import { Menu, type Box, type MenuItem, type MenuState } from './Menu.tsx';
import type { Prompt } from './PromptOverlay.tsx';

// PresetBar::resized
function layOut({ left, top, width, height }: Box) {
  const at = (x: number, w: number): Box => ({ left: x, top, width: w, height });
  const more = at(left + width - 28, 28);
  const save = at(more.left - 4 - 48, 48);
  const previous = at(left, 24);
  const next = at(save.left - 8 - 24, 24);
  const nameLeft = previous.left + previous.width + 4;
  return { previous, next, save, more, name: at(nameLeft, next.left - 4 - nameLeft) };
}

interface PresetBarProps {
  presets: PresetBackend;
  prompt: Prompt;
  bounds: Box;
}

export function PresetBar({ presets, prompt, bounds }: PresetBarProps) {
  const status = useSyncExternalStore(presets.onStatus, presets.status);
  const [menu, setMenu] = useState<MenuState | null>(null);
  const nameButton = useRef<HTMLButtonElement>(null);
  const moreButton = useRef<HTMLButtonElement>(null);
  const layout = layOut(bounds);
  const closeMenu = useCallback(() => setMenu(null), []);

  const report = (error: string | null) => {
    if (error) prompt.showMessage('Preset', error);
  };

  const currentUserPreset = async () => {
    const current = (await presets.list()).find((preset) => isSamePreset(preset, presets.status().current));
    return current && !current.isFactory ? current : undefined;
  };

  const saveAs = () => {
    // Factory names are reserved, so don't suggest one
    const { current } = presets.status();

    prompt.askForText('Save Preset', current.isFactory ? '' : current.name, 'Save', async (name) => {
      const save = async () => report(await presets.save(name, presets.status().current.category));
      if (!(await presets.exists(name))) return save();
      prompt.askToConfirm('Replace Preset', `"${name.trim()}" already exists. Replace it?`, 'Replace', save);
    });
  };

  // User presets save in place; anything else needs a name first
  const save = async () => {
    const user = await currentUserPreset();
    if (user) report(await presets.save(user.name, user.category));
    else saveAs();
  };

  const rename = (preset: PresetEntry) =>
    prompt.askForText('Rename Preset', preset.name, 'Rename', async (name) => report(await presets.rename(preset, name)));

  const remove = (preset: PresetEntry) =>
    prompt.askToConfirm('Delete Preset', `Delete "${preset.name}"?`, 'Delete', async () => report(await presets.remove(preset)));

  const toggle = (show: () => Promise<MenuState>) => () => {
    if (menu) setMenu(null);
    else show().then(setMenu);
  };

  const showPresetMenu = toggle(async () => {
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

    return { items, target: layout.name, owner: nameButton.current };
  });

  const showMoreMenu = toggle(async () => {
    const user = await currentUserPreset();
    const items: MenuItem[] = [
      { kind: 'item', label: 'Init Preset', onSelect: () => void presets.loadInit() },
      { kind: 'separator' },
      { kind: 'item', label: 'Save As...', onSelect: saveAs },
      { kind: 'item', label: 'Rename...', disabled: !user, onSelect: () => user && rename(user) },
      { kind: 'item', label: 'Delete', disabled: !user, onSelect: () => user && remove(user) },
    ];
    if (presets.showFolder) items.push({ kind: 'separator' }, { kind: 'item', label: 'Show Preset Folder', onSelect: presets.showFolder });

    return { items, target: layout.more, owner: moreButton.current };
  });

  return (
    <>
      <TextButton text="<" style={layout.previous} onClick={() => void presets.loadPrevious()} />
      <TextButton ref={nameButton} text={status.current.name + (status.modified ? ' *' : '')} style={layout.name} onClick={showPresetMenu} />
      <TextButton text=">" style={layout.next} onClick={() => void presets.loadNext()} />
      <TextButton text="Save" style={layout.save} onClick={() => void save()} />
      <TextButton ref={moreButton} text="..." style={layout.more} onClick={showMoreMenu} />
      {menu && <Menu {...menu} onClose={closeMenu} />}
    </>
  );
}
