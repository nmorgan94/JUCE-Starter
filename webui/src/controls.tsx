// Mirrors juce::Slider, juce::TextButton and how LookAndFeel_V4 draws them
import { forwardRef, useRef, useState, type CSSProperties, type PointerEvent as ReactPointerEvent } from 'react';
import type { PluginBackend } from './backend.ts';
import { juceHeight } from './fonts.ts';
import { useParam } from './hooks.ts';
import { formatValue, fromProportion, parseValue, toProportion } from './params.ts';

// juce::Slider's defaults
const ROTARY_START = 1.2 * Math.PI;
const ROTARY_END = 2.8 * Math.PI;
const PIXELS_FOR_FULL_DRAG = 250;

// Angles run clockwise from 12 o'clock, as in juce::Path::addCentredArc
const pointAt = (cx: number, cy: number, radius: number, angle: number) =>
  [cx + radius * Math.sin(angle), cy - radius * Math.cos(angle)] as const;

const arcPath = (cx: number, cy: number, radius: number, from: number, to: number) => {
  const [x1, y1] = pointAt(cx, cy, radius, from);
  const [x2, y2] = pointAt(cx, cy, radius, to);
  return `M ${x1} ${y1} A ${radius} ${radius} 0 ${to - from > Math.PI ? 1 : 0} 1 ${x2} ${y2}`;
};

function RotaryGraphic({ width, height, proportion }: { width: number; height: number; proportion: number }) {
  const inner = { width: width - 20, height: height - 20 };
  const cx = width / 2;
  const cy = height / 2;
  const radius = Math.min(inner.width, inner.height) / 2;
  const lineWidth = Math.min(8, radius * 0.5);
  const arcRadius = radius - lineWidth * 0.5;
  const toAngle = ROTARY_START + proportion * (ROTARY_END - ROTARY_START);
  const [dotX, dotY] = pointAt(cx, cy, arcRadius, toAngle);

  return (
    <svg width={width} height={height} aria-hidden>
      <path d={arcPath(cx, cy, arcRadius, ROTARY_START, ROTARY_END)} stroke="var(--color-widget)" strokeWidth={lineWidth} strokeLinecap="round" fill="none" />
      {proportion > 0 && (
        <path d={arcPath(cx, cy, arcRadius, ROTARY_START, toAngle)} stroke="var(--color-thumb)" strokeWidth={lineWidth} strokeLinecap="round" fill="none" />
      )}
      <circle cx={dotX} cy={dotY} r={lineWidth * 0.35} fill="var(--color-fg)" />
    </svg>
  );
}

type DragStart = { x: number; y: number; proportion: number };

interface RotaryProps {
  backend: PluginBackend;
  id: string;
  width: number;
  height: number;
  textBox: { width: number; height: number };
}

// A Slider::RotaryHorizontalVerticalDrag with its text box below
export function Rotary({ backend, id, width, height, textBox }: RotaryProps) {
  const { value, set, commit, param } = useParam(backend, id);
  const drag = useRef<DragStart | null>(null);
  const [editing, setEditing] = useState(false);
  const proportion = toProportion(param, value);
  const text = param.unit ? `${formatValue(param, value)} ${param.unit}` : formatValue(param, value);

  const end = () => {
    if (!drag.current) return;
    drag.current = null;
    backend.endGesture(id);
  };

  const commitText = (typed: string) => {
    setEditing(false);
    const parsed = parseValue(param, typed);
    if (parsed !== null) commit(parsed);
  };

  const box = 'absolute text-center leading-none';
  const boxStyle = { left: (width - textBox.width) / 2, top: height - textBox.height, width: textBox.width, height: textBox.height, fontSize: juceHeight(15) };

  return (
    <div className="relative" style={{ width, height }}>
      <div
        className="cursor-pointer touch-none"
        role="slider"
        aria-label={param.name}
        aria-valuemin={param.min}
        aria-valuemax={param.max}
        aria-valuenow={value}
        aria-valuetext={text}
        onPointerDown={(e: ReactPointerEvent) => {
          if (e.button !== 0) return;
          e.currentTarget.setPointerCapture(e.pointerId);
          drag.current = { x: e.clientX, y: e.clientY, proportion };
          backend.beginGesture(id);
        }}
        onPointerMove={(e: ReactPointerEvent) => {
          const start = drag.current;
          if (!start) return;
          const moved = e.clientX - start.x - (e.clientY - start.y);
          const next = fromProportion(param, start.proportion + moved / PIXELS_FOR_FULL_DRAG);
          if (next !== value) set(next);
        }}
        onPointerUp={end}
        onPointerCancel={end}
        // As SliderAttachment's setDoubleClickReturnValue
        onDoubleClick={() => commit(param.defaultValue)}
      >
        <RotaryGraphic width={width} height={height - textBox.height} proportion={proportion} />
      </div>
      {editing ? (
        <input
          className={`${box} border border-line bg-widget text-fg outline-none`}
          style={boxStyle}
          autoFocus
          defaultValue={formatValue(param, value)}
          onFocus={(e) => e.currentTarget.select()}
          onBlur={(e) => commitText(e.currentTarget.value)}
          onKeyDown={(e) => {
            if (e.key === 'Enter') e.currentTarget.blur();
            if (e.key === 'Escape') setEditing(false);
          }}
        />
      ) : (
        <div className={`${box} flex cursor-text items-center justify-center border border-line`} style={boxStyle} onClick={() => setEditing(true)}>
          {text}
        </div>
      )}
    </div>
  );
}

interface TextButtonProps {
  text: string;
  style: CSSProperties;
  onClick(): void;
}

// LookAndFeel_V4::drawButtonBackground lightens by 5% on hover and 20% while down
export const TextButton = forwardRef<HTMLButtonElement, TextButtonProps>(function TextButton({ text, style, onClick }, ref) {
  return (
    <button
      ref={ref}
      type="button"
      className="absolute cursor-pointer overflow-hidden rounded-md border border-line bg-widget px-1 text-ellipsis whitespace-nowrap text-fg hover:bg-[color-mix(in_srgb,var(--color-widget),white_5%)] active:bg-[color-mix(in_srgb,var(--color-widget),white_20%)]"
      style={{ fontSize: juceHeight(12), ...style }}
      onClick={onClick}
    >
      {text}
    </button>
  );
});
