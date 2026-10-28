// Mirrors Source/ui/PromptOverlay.h
import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { TextButton } from './controls.tsx';
import { juceHeight } from './fonts.ts';

export interface Prompt {
  askForText(title: string, initialText: string, okText: string, onOk: (text: string) => void): void;
  askToConfirm(title: string, message: string, okText: string, onOk: () => void): void;
  showMessage(title: string, message: string): void;
}

interface Shown {
  // Each prompt starts afresh, even one that replaces another
  id: number;
  title: string;
  message: string;
  okText: string;
  initialText: string | null;
  canCancel: boolean;
  onOk: ((text: string) => void) | null;
}

const PANEL = { width: 380, height: 160 };
const BUTTON = { width: 90, height: 28 };

function Overlay({ shown, dismiss }: { shown: Shown; dismiss(): void }) {
  const input = useRef<HTMLInputElement>(null);
  const panel = useRef<HTMLDivElement>(null);

  // Closes first, so the callback can open another prompt
  const accept = () => {
    const text = input.current?.value ?? '';
    dismiss();
    shown.onOk?.(text);
  };

  useEffect(() => {
    if (input.current) {
      input.current.focus();
      input.current.select();
    } else panel.current?.focus();
  }, [shown]);

  return (
    <div
      className="absolute inset-0 z-50 flex items-center justify-center bg-background/75"
      onPointerDown={(e) => e.stopPropagation()}
      onKeyDown={(e) => {
        // Modal: nothing behind the overlay gets keys
        e.stopPropagation();
        if (e.key === 'Escape') dismiss();
        // A focused button handles Enter itself
        if (e.key === 'Enter' && !(e.target instanceof HTMLButtonElement)) accept();
      }}
    >
      <div
        ref={panel}
        tabIndex={-1}
        role="dialog"
        aria-label={shown.title}
        className="relative rounded-lg border border-line bg-widget outline-none"
        style={PANEL}
      >
        <div className="absolute flex items-center font-bold" style={{ left: 20, top: 16, right: 20, height: 24, fontSize: juceHeight(14) }}>
          {shown.title}
        </div>
        {shown.initialText === null ? (
          <div className="absolute overflow-hidden" style={{ left: 20, top: 48, right: 20, height: 36, fontSize: juceHeight(12) }}>
            {shown.message}
          </div>
        ) : (
          <input
            ref={input}
            className="absolute border border-line bg-background px-1 text-fg outline-none focus:border-thumb"
            style={{ left: 20, top: 48, width: PANEL.width - 40, height: 28, fontSize: juceHeight(14) }}
            defaultValue={shown.initialText}
          />
        )}
        {shown.canCancel && (
          <TextButton text="Cancel" style={{ right: 20 + BUTTON.width + 8, bottom: 16, ...BUTTON }} onClick={dismiss} />
        )}
        <TextButton text={shown.okText} style={{ right: 20, bottom: 16, ...BUTTON }} onClick={accept} />
      </div>
    </div>
  );
}

export function usePrompt() {
  const [shown, setShown] = useState<Shown | null>(null);
  const count = useRef(0);
  const dismiss = useCallback(() => setShown(null), []);

  const prompt = useMemo<Prompt>(() => {
    const show = (prompt: Omit<Shown, 'id'>) => setShown({ ...prompt, id: ++count.current });

    return {
      askForText: (title, initialText, okText, onOk) => show({ title, message: '', okText, initialText, canCancel: true, onOk }),
      askToConfirm: (title, message, okText, onOk) => show({ title, message, okText, initialText: null, canCancel: true, onOk }),
      showMessage: (title, message) => show({ title, message, okText: 'OK', initialText: null, canCancel: false, onOk: null }),
    };
  }, []);

  const overlay = shown && <Overlay key={shown.id} shown={shown} dismiss={dismiss} />;
  return { prompt, overlay };
}
