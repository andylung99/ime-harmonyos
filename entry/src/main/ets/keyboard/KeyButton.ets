@Component
export struct KeyButton {
  id: string = '';
  label: string = '';
  col: number = 0;
  row: number = 0;
  keyW: number = 32;
  keyH: number = 50;
  wRatio: number = 1;
  onLocalTap?: (id: string, lx: number, ly: number) => void;

  private onTouch(event: TouchEvent): void {
    const touch = event.touches[0];
    if (touch) {
      this.onLocalTap?.(this.id, touch.x, touch.y);
    }
  }

  build() {
    Button(this.label)
      .width(this.keyW * this.wRatio)
      .height(this.keyH)
      .position({ x: this.col * this.keyW, y: this.row * this.keyH })
      .fontSize(17)
      .backgroundColor('#F1F3F5')
      .fontColor('#111111')
      .borderRadius(6)
      .onTouch((event: TouchEvent) => this.onTouch(event));
  }
}