import { KeyButton } from './KeyButton';
import { buildKeyLayout, KEY_H, KeyItem } from './KeyLayout';

@Component
export struct KeyboardView {
  onTap?: (id: string, x: number, y: number) => void;
  private panelW: number = 360;
  private panelH: number = KEY_H * 4;

  private updatePanel(w: number): void {
    if (w > 0) {
      this.panelW = w;
    }
  }

  build() {
    Stack() {
      ForEach(buildKeyLayout(), (item: KeyItem) => {
        KeyButton({
          id: item.id,
          label: item.label,
          col: item.col,
          row: item.row,
          keyW: this.panelW / 10,
          keyH: KEY_H,
          wRatio: item.wRatio,
          onLocalTap: (id: string, lx: number, ly: number) => {
            this.onTap?.(id, item.col * (this.panelW / 10) + lx, item.row * KEY_H + ly);
          }
        })
      }, (item: KeyItem) => item.id)
    }
    .width('100%')
    .height(this.panelH)
    .backgroundColor('#FFFFFF')
    .onAreaChange((oldV: Area, newV: Area) => {
      this.updatePanel(newV.width as number);
    });
  }
}